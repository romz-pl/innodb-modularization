#include <innodb/page/page_zip_compress_clust.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/flags.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/LOGFILE.h>
#include <innodb/page/page_zip_compress_clust_ext.h>
#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_extra_size.h>


/** Compress the records of a leaf node of a clustered index.
 @return Z_OK, or a zlib error code */
int page_zip_compress_clust(
    FILE_LOGFILE z_stream *c_stream, /*!< in/out: compressed page stream */
    const rec_t **recs,              /*!< in: dense page directory
                                     sorted by address */
    ulint n_dense,                   /*!< in: size of recs[] */
    dict_index_t *index,             /*!< in: the index of the page */
    ulint *n_blobs,                  /*!< in: 0; out: number of
                                     externally stored columns */
    ulint trx_id_col,                /*!< index of the trx_id column */
    byte *deleted,                   /*!< in: dense directory entry pointing
                                     to the head of the free list */
    byte *storage,                   /*!< in: end of dense page directory */
    mem_heap_t *heap)                /*!< in: temporary memory heap */
{
  int err = Z_OK;
  ulint *offsets = NULL;
  /* BTR_EXTERN_FIELD_REF storage */
  byte *externs = storage - n_dense * (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

  ut_ad(*n_blobs == 0);

  do {
    const rec_t *rec = *recs++;

    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);
    ut_ad(rec_offs_n_fields(offsets) == dict_index_get_n_fields(index));
    UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
    UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                       rec_offs_extra_size(offsets));

    /* Compress the extra bytes. */
    c_stream->avail_in =
        static_cast<uInt>(rec - REC_N_NEW_EXTRA_BYTES - c_stream->next_in);

    if (c_stream->avail_in) {
      err = deflate(c_stream, Z_NO_FLUSH);
      if (UNIV_UNLIKELY(err != Z_OK)) {
        goto func_exit;
      }
    }
    ut_ad(!c_stream->avail_in);
    ut_ad(c_stream->next_in == rec - REC_N_NEW_EXTRA_BYTES);

    /* Compress the data bytes. */

    c_stream->next_in = (byte *)rec;

    /* Check if there are any externally stored columns.
    For each externally stored column, store the
    BTR_EXTERN_FIELD_REF separately. */
    if (rec_offs_any_extern(offsets)) {
      ut_ad(index->is_clustered());

      err = page_zip_compress_clust_ext(LOGFILE c_stream, rec, offsets,
                                        trx_id_col, deleted, storage, &externs,
                                        n_blobs);

      if (UNIV_UNLIKELY(err != Z_OK)) {
        goto func_exit;
      }
    } else {
      ulint len;
      const byte *src;

      /* Store trx_id and roll_ptr in uncompressed form. */
      src = rec_get_nth_field(rec, offsets, trx_id_col, &len);
      ut_ad(src + DATA_TRX_ID_LEN ==
            rec_get_nth_field(rec, offsets, trx_id_col + 1, &len));
      ut_ad(len == DATA_ROLL_PTR_LEN);
      UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
      UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                         rec_offs_extra_size(offsets));

      /* Compress any preceding bytes. */
      c_stream->avail_in = static_cast<uInt>(src - c_stream->next_in);

      if (c_stream->avail_in) {
        err = deflate(c_stream, Z_NO_FLUSH);
        if (UNIV_UNLIKELY(err != Z_OK)) {
          return (err);
        }
      }

      ut_ad(!c_stream->avail_in);
      ut_ad(c_stream->next_in == src);

      memcpy(storage - (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN) *
                           (rec_get_heap_no_new(rec) - 1),
             c_stream->next_in, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

      c_stream->next_in += DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN;

      /* Skip also roll_ptr */
      ut_ad(trx_id_col + 1 < rec_offs_n_fields(offsets));
    }

    /* Compress the last bytes of the record. */
    c_stream->avail_in = static_cast<uInt>(rec + rec_offs_data_size(offsets) -
                                           c_stream->next_in);

    if (c_stream->avail_in) {
      err = deflate(c_stream, Z_NO_FLUSH);
      if (UNIV_UNLIKELY(err != Z_OK)) {
        goto func_exit;
      }
    }
    ut_ad(!c_stream->avail_in);
  } while (--n_dense);

func_exit:
  return (err);
}
