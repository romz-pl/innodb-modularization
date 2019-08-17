#include <innodb/page/page_zip_compress_clust_ext.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/flags.h>
#include <innodb/disk/flags.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_zip_dir_find_low.h>
#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_extern.h>


/** Compress a record of a leaf node of a clustered index that contains
 externally stored columns.
 @return Z_OK, or a zlib error code */
int page_zip_compress_clust_ext(
    FILE_LOGFILE z_stream *c_stream, /*!< in/out: compressed page stream */
    const rec_t *rec,                /*!< in: record */
    const ulint *offsets,            /*!< in: rec_get_offsets(rec) */
    ulint trx_id_col,                /*!< in: position of of DB_TRX_ID */
    byte *deleted,                   /*!< in: dense directory entry pointing
                                     to the head of the free list */
    byte *storage,                   /*!< in: end of dense page directory */
    byte **externs,                  /*!< in/out: pointer to the next
                                     available BLOB pointer */
    ulint *n_blobs)                  /*!< in/out: number of
                                     externally stored columns */
{
  int err;
  ulint i;

  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  for (i = 0; i < rec_offs_n_fields(offsets); i++) {
    ulint len;
    const byte *src;

    if (UNIV_UNLIKELY(i == trx_id_col)) {
      ut_ad(!rec_offs_nth_extern(offsets, i));
      /* Store trx_id and roll_ptr
      in uncompressed form. */
      src = rec_get_nth_field(rec, offsets, i, &len);
      ut_ad(src + DATA_TRX_ID_LEN ==
            rec_get_nth_field(rec, offsets, i + 1, &len));
      ut_ad(len == DATA_ROLL_PTR_LEN);

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
      i++;
    } else if (rec_offs_nth_extern(offsets, i)) {
      src = rec_get_nth_field(rec, offsets, i, &len);
      ut_ad(len >= BTR_EXTERN_FIELD_REF_SIZE);
      src += len - BTR_EXTERN_FIELD_REF_SIZE;

      c_stream->avail_in = static_cast<uInt>(src - c_stream->next_in);
      if (UNIV_LIKELY(c_stream->avail_in)) {
        err = deflate(c_stream, Z_NO_FLUSH);
        if (UNIV_UNLIKELY(err != Z_OK)) {
          return (err);
        }
      }

      ut_ad(!c_stream->avail_in);
      ut_ad(c_stream->next_in == src);

      /* Reserve space for the data at
      the end of the space reserved for
      the compressed data and the page
      modification log. */

      if (UNIV_UNLIKELY(c_stream->avail_out <= BTR_EXTERN_FIELD_REF_SIZE)) {
        /* out of space */
        return (Z_BUF_ERROR);
      }

      ut_ad(*externs == c_stream->next_out + c_stream->avail_out +
                            1 /* end of modif. log */);

      c_stream->next_in += BTR_EXTERN_FIELD_REF_SIZE;

      /* Skip deleted records. */
      if (UNIV_LIKELY_NULL(
              page_zip_dir_find_low(storage, deleted, page_offset(rec)))) {
        continue;
      }

      (*n_blobs)++;
      c_stream->avail_out -= BTR_EXTERN_FIELD_REF_SIZE;
      *externs -= BTR_EXTERN_FIELD_REF_SIZE;

      /* Copy the BLOB pointer */
      memcpy(*externs, c_stream->next_in - BTR_EXTERN_FIELD_REF_SIZE,
             BTR_EXTERN_FIELD_REF_SIZE);
    }
  }

  return (Z_OK);
}
