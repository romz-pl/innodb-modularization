#include <innodb/page/page_zip_apply_log.h>

#include <innodb/compiler_hints/compiler_hints.h>
#include <innodb/machine/data.h>
#include <innodb/memory/mem_heap_free.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/page/page_zip_apply_log_ext.h>
#include <innodb/page/page_zip_fail.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_end.h>
#include <innodb/record/rec_get_nth_field_offs.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_get_offsets_reverse.h>
#include <innodb/record/rec_get_start.h>
#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_make_valid.h>


/** Apply the modification log to an uncompressed page.
 Do not copy the fields that are stored separately.
 @return pointer to end of modification log, or NULL on failure */
const byte *page_zip_apply_log(
    const byte *data, /*!< in: modification log */
    ulint size,       /*!< in: maximum length of the log, in bytes */
    rec_t **recs,     /*!< in: dense page directory,
                      sorted by address (indexed by
                      heap_no - PAGE_HEAP_NO_USER_LOW) */
    ulint n_dense,    /*!< in: size of recs[] */
    ulint trx_id_col, /*!< in: column number of trx_id in the index,
                   or ULINT_UNDEFINED if none */
    ulint heap_status,
    /*!< in: heap_no and status bits for
    the next record to uncompress */
    dict_index_t *index, /*!< in: index of the page */
    ulint *offsets)      /*!< in/out: work area for
                         rec_get_offsets_reverse() */
{
  const byte *const end = data + size;

  for (;;) {
    ulint val;
    rec_t *rec;
    ulint len;
    ulint hs;

    val = *data++;
    if (UNIV_UNLIKELY(!val)) {
      return (data - 1);
    }
    if (val & 0x80) {
      val = (val & 0x7f) << 8 | *data++;
      if (UNIV_UNLIKELY(!val)) {
        page_zip_fail(
            ("page_zip_apply_log:"
             " invalid val %x%x\n",
             data[-2], data[-1]));
        return (NULL);
      }
    }
    if (UNIV_UNLIKELY(data >= end)) {
      page_zip_fail(("page_zip_apply_log: %p >= %p\n", (const void *)data,
                     (const void *)end));
      return (NULL);
    }
    if (UNIV_UNLIKELY((val >> 1) > n_dense)) {
      page_zip_fail(
          ("page_zip_apply_log: %lu>>1 > %lu\n", (ulong)val, (ulong)n_dense));
      return (NULL);
    }

    /* Determine the heap number and status bits of the record. */
    rec = recs[(val >> 1) - 1];

    hs = ((val >> 1) + 1) << REC_HEAP_NO_SHIFT;
    hs |= heap_status & ((1 << REC_HEAP_NO_SHIFT) - 1);

    /* This may either be an old record that is being
    overwritten (updated in place, or allocated from
    the free list), or a new record, with the next
    available_heap_no. */
    if (UNIV_UNLIKELY(hs > heap_status)) {
      page_zip_fail(
          ("page_zip_apply_log: %lu > %lu\n", (ulong)hs, (ulong)heap_status));
      return (NULL);
    } else if (hs == heap_status) {
      /* A new record was allocated from the heap. */
      if (UNIV_UNLIKELY(val & 1)) {
        /* Only existing records may be cleared. */
        page_zip_fail(
            ("page_zip_apply_log:"
             " attempting to create"
             " deleted rec %lu\n",
             (ulong)hs));
        return (NULL);
      }
      heap_status += 1 << REC_HEAP_NO_SHIFT;
    }

    mach_write_to_2(rec - REC_NEW_HEAP_NO, hs);

    if (val & 1) {
      /* Clear the data bytes of the record. */
      mem_heap_t *heap = NULL;
      ulint *offs;
      offs = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);
      memset(rec, 0, rec_offs_data_size(offs));

      if (UNIV_LIKELY_NULL(heap)) {
        mem_heap_free(heap);
      }
      continue;
    }

#if REC_STATUS_NODE_PTR != TRUE
#error "REC_STATUS_NODE_PTR != TRUE"
#endif
    rec_get_offsets_reverse(data, index, hs & REC_STATUS_NODE_PTR, offsets);
    rec_offs_make_valid(rec, index, offsets);

    /* Copy the extra bytes (backwards). */
    {
      byte *start = rec_get_start(rec, offsets);
      byte *b = rec - REC_N_NEW_EXTRA_BYTES;
      while (b != start) {
        *--b = *data++;
      }
    }

    /* Copy the data bytes. */
    if (UNIV_UNLIKELY(rec_offs_any_extern(offsets))) {
      /* Non-leaf nodes should not contain any
      externally stored columns. */
      if (UNIV_UNLIKELY(hs & REC_STATUS_NODE_PTR)) {
        page_zip_fail(
            ("page_zip_apply_log:"
             " %lu&REC_STATUS_NODE_PTR\n",
             (ulong)hs));
        return (NULL);
      }

      data = page_zip_apply_log_ext(rec, offsets, trx_id_col, data, end);

      if (UNIV_UNLIKELY(!data)) {
        return (NULL);
      }
    } else if (UNIV_UNLIKELY(hs & REC_STATUS_NODE_PTR)) {
      len = rec_offs_data_size(offsets) - REC_NODE_PTR_SIZE;
      /* Copy the data bytes, except node_ptr. */
      if (UNIV_UNLIKELY(data + len >= end)) {
        page_zip_fail(
            ("page_zip_apply_log:"
             " node_ptr %p+%lu >= %p\n",
             (const void *)data, (ulong)len, (const void *)end));
        return (NULL);
      }
      memcpy(rec, data, len);
      data += len;
    } else if (UNIV_LIKELY(trx_id_col == ULINT_UNDEFINED)) {
      len = rec_offs_data_size(offsets);

      /* Copy all data bytes of
      a record in a secondary index. */
      if (UNIV_UNLIKELY(data + len >= end)) {
        page_zip_fail(
            ("page_zip_apply_log:"
             " sec %p+%lu >= %p\n",
             (const void *)data, (ulong)len, (const void *)end));
        return (NULL);
      }

      memcpy(rec, data, len);
      data += len;
    } else {
      /* Skip DB_TRX_ID and DB_ROLL_PTR. */
      ulint l = rec_get_nth_field_offs(offsets, trx_id_col, &len);
      byte *b;

      if (UNIV_UNLIKELY(data + l >= end) ||
          UNIV_UNLIKELY(len < (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN))) {
        page_zip_fail(
            ("page_zip_apply_log:"
             " trx_id %p+%lu >= %p\n",
             (const void *)data, (ulong)l, (const void *)end));
        return (NULL);
      }

      /* Copy any preceding data bytes. */
      memcpy(rec, data, l);
      data += l;

      /* Copy any bytes following DB_TRX_ID, DB_ROLL_PTR. */
      b = rec + l + (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);
      len = rec_get_end(rec, offsets) - b;
      if (UNIV_UNLIKELY(data + len >= end)) {
        page_zip_fail(
            ("page_zip_apply_log:"
             " clust %p+%lu >= %p\n",
             (const void *)data, (ulong)len, (const void *)end));
        return (NULL);
      }
      memcpy(b, data, len);
      data += len;
    }
  }
}
