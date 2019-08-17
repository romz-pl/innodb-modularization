#include <innodb/page/page_zip_decompress_node_ptrs.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/disk/page_t.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_zip_apply_log.h>
#include <innodb/page/page_zip_decompress_heap_no.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_start_low.h>
#include <innodb/page/page_zip_fail.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_get_trailer_len.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_end.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/page/page_header_get_field.h>
#include <innodb/page/page_dir_get_n_slots.h>



/** Decompress the records of a node pointer page.
 @return true on success, false on failure */
ibool page_zip_decompress_node_ptrs(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    z_stream *d_stream,       /*!< in/out: compressed page stream */
    rec_t **recs,             /*!< in: dense page directory
                              sorted by address */
    ulint n_dense,            /*!< in: size of recs[] */
    dict_index_t *index,      /*!< in: the index of the page */
    ulint *offsets,           /*!< in/out: temporary offsets */
    mem_heap_t *heap)         /*!< in: temporary memory heap */
{
  ulint heap_status = REC_STATUS_NODE_PTR | PAGE_HEAP_NO_USER_LOW
                                                << REC_HEAP_NO_SHIFT;
  ulint slot;
  const byte *storage;

  /* Subtract the space reserved for uncompressed data. */
  d_stream->avail_in -=
      static_cast<uInt>(n_dense * (PAGE_ZIP_DIR_SLOT_SIZE + REC_NODE_PTR_SIZE));

  /* Decompress the records in heap_no order. */
  for (slot = 0; slot < n_dense; slot++) {
    rec_t *rec = recs[slot];

    d_stream->avail_out =
        static_cast<uInt>(rec - REC_N_NEW_EXTRA_BYTES - d_stream->next_out);

    ut_ad(d_stream->avail_out < UNIV_PAGE_SIZE - PAGE_ZIP_START - PAGE_DIR);
    switch (inflate(d_stream, Z_SYNC_FLUSH)) {
      case Z_STREAM_END:
        page_zip_decompress_heap_no(d_stream, rec, heap_status);
        goto zlib_done;
      case Z_OK:
      case Z_BUF_ERROR:
        if (!d_stream->avail_out) {
          break;
        }
        /* fall through */
      default:
        page_zip_fail(
            ("page_zip_decompress_node_ptrs:"
             " 1 inflate(Z_SYNC_FLUSH)=%s\n",
             d_stream->msg));
        goto zlib_error;
    }

    if (!page_zip_decompress_heap_no(d_stream, rec, heap_status)) {
      ut_ad(0);
    }

    /* Read the offsets. The status bits are needed here. */
    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);

    /* Non-leaf nodes should not have any externally
    stored columns. */
    ut_ad(!rec_offs_any_extern(offsets));

    /* Decompress the data bytes, except node_ptr. */
    d_stream->avail_out =
        static_cast<uInt>(rec_offs_data_size(offsets) - REC_NODE_PTR_SIZE);

    switch (inflate(d_stream, Z_SYNC_FLUSH)) {
      case Z_STREAM_END:
        goto zlib_done;
      case Z_OK:
      case Z_BUF_ERROR:
        if (!d_stream->avail_out) {
          break;
        }
        /* fall through */
      default:
        page_zip_fail(
            ("page_zip_decompress_node_ptrs:"
             " 2 inflate(Z_SYNC_FLUSH)=%s\n",
             d_stream->msg));
        goto zlib_error;
    }

    /* Clear the node pointer in case the record
    will be deleted and the space will be reallocated
    to a smaller record. */
    memset(d_stream->next_out, 0, REC_NODE_PTR_SIZE);
    d_stream->next_out += REC_NODE_PTR_SIZE;

    ut_ad(d_stream->next_out == rec_get_end(rec, offsets));
  }

  /* Decompress any trailing garbage, in case the last record was
  allocated from an originally longer space on the free list. */
  d_stream->avail_out =
      static_cast<uInt>(page_header_get_field(page_zip->data, PAGE_HEAP_TOP) -
                        page_offset(d_stream->next_out));
  if (UNIV_UNLIKELY(d_stream->avail_out >
                    UNIV_PAGE_SIZE - PAGE_ZIP_START - PAGE_DIR)) {
    page_zip_fail(
        ("page_zip_decompress_node_ptrs:"
         " avail_out = %u\n",
         d_stream->avail_out));
    goto zlib_error;
  }

  if (UNIV_UNLIKELY(inflate(d_stream, Z_FINISH) != Z_STREAM_END)) {
    page_zip_fail(
        ("page_zip_decompress_node_ptrs:"
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
    mod_log_ptr = page_zip_apply_log(d_stream->next_in, d_stream->avail_in + 1,
                                     recs, n_dense, ULINT_UNDEFINED,
                                     heap_status, index, offsets);

    if (UNIV_UNLIKELY(!mod_log_ptr)) {
      return (FALSE);
    }
    page_zip->m_end = mod_log_ptr - page_zip->data;
    page_zip->m_nonempty = mod_log_ptr != d_stream->next_in;
  }

  if (UNIV_UNLIKELY(page_zip_get_trailer_len(page_zip, index->is_clustered()) +
                        page_zip->m_end >=
                    page_zip_get_size(page_zip))) {
    page_zip_fail(
        ("page_zip_decompress_node_ptrs:"
         " %lu + %lu >= %lu, %lu\n",
         (ulong)page_zip_get_trailer_len(page_zip, index->is_clustered()),
         (ulong)page_zip->m_end, (ulong)page_zip_get_size(page_zip),
         (ulong)index->is_clustered()));
    return (FALSE);
  }

  /* Restore the uncompressed columns in heap_no order. */
  storage = page_zip_dir_start_low(page_zip, n_dense);

  for (slot = 0; slot < n_dense; slot++) {
    rec_t *rec = recs[slot];

    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);
    /* Non-leaf nodes should not have any externally
    stored columns. */
    ut_ad(!rec_offs_any_extern(offsets));
    storage -= REC_NODE_PTR_SIZE;

    memcpy(rec_get_end(rec, offsets) - REC_NODE_PTR_SIZE, storage,
           REC_NODE_PTR_SIZE);
  }

  return (TRUE);
}

