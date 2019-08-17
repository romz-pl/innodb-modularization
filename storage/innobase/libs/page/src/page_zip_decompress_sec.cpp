#include <innodb/page/page_zip_decompress_sec.h>

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

/** Decompress the records of a leaf node of a secondary index.
 @return true on success, false on failure */
ibool page_zip_decompress_sec(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    z_stream *d_stream,       /*!< in/out: compressed page stream */
    rec_t **recs,             /*!< in: dense page directory
                              sorted by address */
    ulint n_dense,            /*!< in: size of recs[] */
    dict_index_t *index,      /*!< in: the index of the page */
    ulint *offsets)           /*!< in/out: temporary offsets */
{
  ulint heap_status = REC_STATUS_ORDINARY | PAGE_HEAP_NO_USER_LOW
                                                << REC_HEAP_NO_SHIFT;
  ulint slot;

  ut_a(!index->is_clustered());

  /* Subtract the space reserved for uncompressed data. */
  d_stream->avail_in -= static_cast<uint>(n_dense * PAGE_ZIP_DIR_SLOT_SIZE);

  for (slot = 0; slot < n_dense; slot++) {
    rec_t *rec = recs[slot];

    /* Decompress everything up to this record. */
    d_stream->avail_out =
        static_cast<uint>(rec - REC_N_NEW_EXTRA_BYTES - d_stream->next_out);

    if (UNIV_LIKELY(d_stream->avail_out)) {
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
              ("page_zip_decompress_sec:"
               " inflate(Z_SYNC_FLUSH)=%s\n",
               d_stream->msg));
          goto zlib_error;
      }
    }

    if (!page_zip_decompress_heap_no(d_stream, rec, heap_status)) {
      ut_ad(0);
    }
  }

  /* Decompress the data of the last record and any trailing garbage,
  in case the last record was allocated from an originally longer space
  on the free list. */
  d_stream->avail_out =
      static_cast<uInt>(page_header_get_field(page_zip->data, PAGE_HEAP_TOP) -
                        page_offset(d_stream->next_out));
  if (UNIV_UNLIKELY(d_stream->avail_out >
                    UNIV_PAGE_SIZE - PAGE_ZIP_START - PAGE_DIR)) {
    page_zip_fail(
        ("page_zip_decompress_sec:"
         " avail_out = %u\n",
         d_stream->avail_out));
    goto zlib_error;
  }

  if (UNIV_UNLIKELY(inflate(d_stream, Z_FINISH) != Z_STREAM_END)) {
    page_zip_fail(
        ("page_zip_decompress_sec:"
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

  if (UNIV_UNLIKELY(page_zip_get_trailer_len(page_zip, FALSE) +
                        page_zip->m_end >=
                    page_zip_get_size(page_zip))) {
    page_zip_fail(("page_zip_decompress_sec: %lu + %lu >= %lu\n",
                   (ulong)page_zip_get_trailer_len(page_zip, FALSE),
                   (ulong)page_zip->m_end, (ulong)page_zip_get_size(page_zip)));
    return (FALSE);
  }

  /* There are no uncompressed columns on leaf pages of
  secondary indexes. */

  return (TRUE);
}
