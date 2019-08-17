#include <innodb/page/page_zip_decompress_clust.h>

#include <innodb/assert/assert.h>
#include <innodb/compiler_hints/compiler_hints.h>
#include <innodb/data_types/flags.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/disk/flags.h>
#include <innodb/disk/page_t.h>
#include <innodb/error/ut_error.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_dir_get_n_slots.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_header_get_field.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_zip_apply_log.h>
#include <innodb/page/page_zip_decompress_clust_ext.h>
#include <innodb/page/page_zip_decompress_heap_no.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_find_free.h>
#include <innodb/page/page_zip_dir_start_low.h>
#include <innodb/page/page_zip_fail.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_get_trailer_len.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_end.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_extern.h>


static_assert(DATA_TRX_ID_LEN <= 6, "COMPRESSION_ALGORITHM will not fit!");


/** Compress the records of a leaf node of a clustered index.
 @return true on success, false on failure */
ibool page_zip_decompress_clust(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    z_stream *d_stream,       /*!< in/out: compressed page stream */
    rec_t **recs,             /*!< in: dense page directory
                              sorted by address */
    ulint n_dense,            /*!< in: size of recs[] */
    dict_index_t *index,      /*!< in: the index of the page */
    ulint trx_id_col,         /*!< index of the trx_id column */
    ulint *offsets,           /*!< in/out: temporary offsets */
    mem_heap_t *heap)         /*!< in: temporary memory heap */
{
  int err;
  ulint slot;
  ulint heap_status = REC_STATUS_ORDINARY | PAGE_HEAP_NO_USER_LOW
                                                << REC_HEAP_NO_SHIFT;
  const byte *storage;
  const byte *externs;

  ut_a(index->is_clustered());

  /* Subtract the space reserved for uncompressed data. */
  d_stream->avail_in -=
      static_cast<uInt>(n_dense) * (PAGE_ZIP_CLUST_LEAF_SLOT_SIZE);

  /* Decompress the records in heap_no order. */
  for (slot = 0; slot < n_dense; slot++) {
    rec_t *rec = recs[slot];

    d_stream->avail_out =
        static_cast<uInt>(rec - REC_N_NEW_EXTRA_BYTES - d_stream->next_out);

    ut_ad(d_stream->avail_out < UNIV_PAGE_SIZE - PAGE_ZIP_START - PAGE_DIR);
    err = inflate(d_stream, Z_SYNC_FLUSH);
    switch (err) {
      case Z_STREAM_END:
        page_zip_decompress_heap_no(d_stream, rec, heap_status);
        goto zlib_done;
      case Z_OK:
      case Z_BUF_ERROR:
        if (UNIV_LIKELY(!d_stream->avail_out)) {
          break;
        }
        /* fall through */
      default:
        page_zip_fail(
            ("page_zip_decompress_clust:"
             " 1 inflate(Z_SYNC_FLUSH)=%s\n",
             d_stream->msg));
        goto zlib_error;
    }

    if (!page_zip_decompress_heap_no(d_stream, rec, heap_status)) {
      ut_ad(0);
    }

    /* Read the offsets. The status bits are needed here. */
    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);

    /* This is a leaf page in a clustered index. */

    /* Check if there are any externally stored columns.
    For each externally stored column, restore the
    BTR_EXTERN_FIELD_REF separately. */

    if (rec_offs_any_extern(offsets)) {
      if (UNIV_UNLIKELY(!page_zip_decompress_clust_ext(d_stream, rec, offsets,
                                                       trx_id_col))) {
        goto zlib_error;
      }
    } else {
      /* Skip trx_id and roll_ptr */
      ulint len;
      byte *dst = rec_get_nth_field(rec, offsets, trx_id_col, &len);
      if (UNIV_UNLIKELY(len < DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN)) {
        page_zip_fail(
            ("page_zip_decompress_clust:"
             " len = %lu\n",
             (ulong)len));
        goto zlib_error;
      }

      d_stream->avail_out = static_cast<uInt>(dst - d_stream->next_out);

      switch (inflate(d_stream, Z_SYNC_FLUSH)) {
        case Z_STREAM_END:
        case Z_OK:
        case Z_BUF_ERROR:
          if (!d_stream->avail_out) {
            break;
          }
          /* fall through */
        default:
          page_zip_fail(
              ("page_zip_decompress_clust:"
               " 2 inflate(Z_SYNC_FLUSH)=%s\n",
               d_stream->msg));
          goto zlib_error;
      }

      ut_ad(d_stream->next_out == dst);

      /* Clear DB_TRX_ID and DB_ROLL_PTR in order to
      avoid uninitialized bytes in case the record
      is affected by page_zip_apply_log(). */
      memset(dst, 0, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

      d_stream->next_out += DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN;
    }

    /* Decompress the last bytes of the record. */
    d_stream->avail_out =
        static_cast<uInt>(rec_get_end(rec, offsets) - d_stream->next_out);

    switch (inflate(d_stream, Z_SYNC_FLUSH)) {
      case Z_STREAM_END:
      case Z_OK:
      case Z_BUF_ERROR:
        if (!d_stream->avail_out) {
          break;
        }
        /* fall through */
      default:
        page_zip_fail(
            ("page_zip_decompress_clust:"
             " 3 inflate(Z_SYNC_FLUSH)=%s\n",
             d_stream->msg));
        goto zlib_error;
    }
  }

  /* Decompress any trailing garbage, in case the last record was
  allocated from an originally longer space on the free list. */
  d_stream->avail_out =
      static_cast<uInt>(page_header_get_field(page_zip->data, PAGE_HEAP_TOP) -
                        page_offset(d_stream->next_out));
  if (UNIV_UNLIKELY(d_stream->avail_out >
                    UNIV_PAGE_SIZE - PAGE_ZIP_START - PAGE_DIR)) {
    page_zip_fail(
        ("page_zip_decompress_clust:"
         " avail_out = %u\n",
         d_stream->avail_out));
    goto zlib_error;
  }

  if (UNIV_UNLIKELY(inflate(d_stream, Z_FINISH) != Z_STREAM_END)) {
    page_zip_fail(
        ("page_zip_decompress_clust:"
         " inflate(Z_FINISH)=%s\n",
         d_stream->msg));
  zlib_error:
    inflateEnd(d_stream);
    return (FALSE);
  }

  /* Note that d_stream->avail_out > 0 may hold here
  if the modification log is nonempty. */

zlib_done:
  if (UNIV_UNLIKELY(inflateEnd(d_stream) != Z_OK)) {
    ut_error;
  }

  {
    page_t *page = page_align(d_stream->next_out);

    /* Clear the unused heap space on the uncompressed page. */
    memset(d_stream->next_out, 0,
           page_dir_get_nth_slot(page, page_dir_get_n_slots(page) - 1) -
               d_stream->next_out);
  }

#ifdef UNIV_DEBUG
  page_zip->m_start = PAGE_DATA + d_stream->total_in;
#endif /* UNIV_DEBUG */

  /* Apply the modification log. */
  {
    const byte *mod_log_ptr;
    mod_log_ptr =
        page_zip_apply_log(d_stream->next_in, d_stream->avail_in + 1, recs,
                           n_dense, trx_id_col, heap_status, index, offsets);

    if (UNIV_UNLIKELY(!mod_log_ptr)) {
      return (FALSE);
    }
    page_zip->m_end = mod_log_ptr - page_zip->data;
    page_zip->m_nonempty = mod_log_ptr != d_stream->next_in;
  }

  if (UNIV_UNLIKELY(page_zip_get_trailer_len(page_zip, TRUE) +
                        page_zip->m_end >=
                    page_zip_get_size(page_zip))) {
    page_zip_fail(("page_zip_decompress_clust: %lu + %lu >= %lu\n",
                   (ulong)page_zip_get_trailer_len(page_zip, TRUE),
                   (ulong)page_zip->m_end, (ulong)page_zip_get_size(page_zip)));
    return (FALSE);
  }

  storage = page_zip_dir_start_low(page_zip, n_dense);

  externs = storage - n_dense * (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

  /* Restore the uncompressed columns in heap_no order. */

  for (slot = 0; slot < n_dense; slot++) {
    ulint i;
    ulint len;
    byte *dst;
    rec_t *rec = recs[slot];
    ibool exists = !page_zip_dir_find_free(page_zip, page_offset(rec));
    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);

    dst = rec_get_nth_field(rec, offsets, trx_id_col, &len);
    ut_ad(len >= DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);
    storage -= DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN;
    memcpy(dst, storage, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

    /* Check if there are any externally stored
    columns in this record.  For each externally
    stored column, restore or clear the
    BTR_EXTERN_FIELD_REF. */
    if (!rec_offs_any_extern(offsets)) {
      continue;
    }

    for (i = 0; i < rec_offs_n_fields(offsets); i++) {
      if (!rec_offs_nth_extern(offsets, i)) {
        continue;
      }
      dst = rec_get_nth_field(rec, offsets, i, &len);

      if (UNIV_UNLIKELY(len < BTR_EXTERN_FIELD_REF_SIZE)) {
        page_zip_fail(
            ("page_zip_decompress_clust:"
             " %lu < 20\n",
             (ulong)len));
        return (FALSE);
      }

      dst += len - BTR_EXTERN_FIELD_REF_SIZE;

      if (UNIV_LIKELY(exists)) {
        /* Existing record:
        restore the BLOB pointer */
        externs -= BTR_EXTERN_FIELD_REF_SIZE;

        if (UNIV_UNLIKELY(externs < page_zip->data + page_zip->m_end)) {
          page_zip_fail(
              ("page_zip_"
               "decompress_clust:"
               " %p < %p + %lu\n",
               (const void *)externs, (const void *)page_zip->data,
               (ulong)page_zip->m_end));
          return (FALSE);
        }

        memcpy(dst, externs, BTR_EXTERN_FIELD_REF_SIZE);

        page_zip->n_blobs++;
      } else {
        /* Deleted record:
        clear the BLOB pointer */
        memset(dst, 0, BTR_EXTERN_FIELD_REF_SIZE);
      }
    }
  }

  return (TRUE);
}

