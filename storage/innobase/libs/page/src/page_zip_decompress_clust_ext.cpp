#include <innodb/page/page_zip_decompress_clust_ext.h>

#include <innodb/assert/assert.h>
#include <innodb/compiler_hints/compiler_hints.h>
#include <innodb/data_types/flags.h>
#include <innodb/disk/flags.h>
#include <innodb/page/page_zip_fail.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_extern.h>


static_assert(DATA_TRX_ID_LEN <= 6, "COMPRESSION_ALGORITHM will not fit!");


/** Decompress a record of a leaf node of a clustered index that contains
 externally stored columns.
 @return true on success */
ibool page_zip_decompress_clust_ext(
    z_stream *d_stream,   /*!< in/out: compressed page stream */
    rec_t *rec,           /*!< in/out: record */
    const ulint *offsets, /*!< in: rec_get_offsets(rec) */
    ulint trx_id_col)     /*!< in: position of of DB_TRX_ID */
{
  ulint i;

  for (i = 0; i < rec_offs_n_fields(offsets); i++) {
    ulint len;
    byte *dst;

    if (UNIV_UNLIKELY(i == trx_id_col)) {
      /* Skip trx_id and roll_ptr */
      dst = rec_get_nth_field(rec, offsets, i, &len);
      if (UNIV_UNLIKELY(len < DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN)) {
        page_zip_fail(
            ("page_zip_decompress_clust_ext:"
             " len[%lu] = %lu\n",
             (ulong)i, (ulong)len));
        return (FALSE);
      }

      if (rec_offs_nth_extern(offsets, i)) {
        page_zip_fail(
            ("page_zip_decompress_clust_ext:"
             " DB_TRX_ID at %lu is ext\n",
             (ulong)i));
        return (FALSE);
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
              ("page_zip_decompress_clust_ext:"
               " 1 inflate(Z_SYNC_FLUSH)=%s\n",
               d_stream->msg));
          return (FALSE);
      }

      ut_ad(d_stream->next_out == dst);

      /* Clear DB_TRX_ID and DB_ROLL_PTR in order to
      avoid uninitialized bytes in case the record
      is affected by page_zip_apply_log(). */
      memset(dst, 0, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

      d_stream->next_out += DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN;
    } else if (rec_offs_nth_extern(offsets, i)) {
      dst = rec_get_nth_field(rec, offsets, i, &len);
      ut_ad(len >= BTR_EXTERN_FIELD_REF_SIZE);
      dst += len - BTR_EXTERN_FIELD_REF_SIZE;

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
              ("page_zip_decompress_clust_ext:"
               " 2 inflate(Z_SYNC_FLUSH)=%s\n",
               d_stream->msg));
          return (FALSE);
      }

      ut_ad(d_stream->next_out == dst);

      /* Clear the BLOB pointer in case
      the record will be deleted and the
      space will not be reused.  Note that
      the final initialization of the BLOB
      pointers (copying from "externs"
      or clearing) will have to take place
      only after the page modification log
      has been applied.  Otherwise, we
      could end up with an uninitialized
      BLOB pointer when a record is deleted,
      reallocated and deleted. */
      memset(d_stream->next_out, 0, BTR_EXTERN_FIELD_REF_SIZE);
      d_stream->next_out += BTR_EXTERN_FIELD_REF_SIZE;
    }
  }

  return (TRUE);
}
