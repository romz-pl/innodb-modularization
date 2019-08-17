#include <innodb/page/page_zip_compress_sec.h>

#include <innodb/assert/assert.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/memory_check/memory_check.h>

/** Compress the records of a leaf node of a secondary index.
 @return Z_OK, or a zlib error code */
int page_zip_compress_sec(
    FILE_LOGFILE z_stream *c_stream, /*!< in/out: compressed page stream */
    const rec_t **recs,              /*!< in: dense page directory
                                     sorted by address */
    ulint n_dense)                   /*!< in: size of recs[] */
{
  int err = Z_OK;

  ut_ad(n_dense > 0);

  do {
    const rec_t *rec = *recs++;

    /* Compress everything up to this record. */
    c_stream->avail_in =
        static_cast<uInt>(rec - REC_N_NEW_EXTRA_BYTES - c_stream->next_in);

    if (UNIV_LIKELY(c_stream->avail_in)) {
      UNIV_MEM_ASSERT_RW(c_stream->next_in, c_stream->avail_in);
      err = deflate(c_stream, Z_NO_FLUSH);
      if (UNIV_UNLIKELY(err != Z_OK)) {
        break;
      }
    }

    ut_ad(!c_stream->avail_in);
    ut_ad(c_stream->next_in == rec - REC_N_NEW_EXTRA_BYTES);

    /* Skip the REC_N_NEW_EXTRA_BYTES. */

    c_stream->next_in = (byte *)rec;
  } while (--n_dense);

  return (err);
}
