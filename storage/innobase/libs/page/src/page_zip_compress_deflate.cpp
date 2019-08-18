#include <innodb/page/page_zip_compress_deflate.h>


#ifdef PAGE_ZIP_COMPRESS_DBG

/** Wrapper for deflate().  Log the operation if page_zip_compress_dbg is set.
 @return deflate() status: Z_OK, Z_BUF_ERROR, ... */
int page_zip_compress_deflate(
    FILE *logfile,  /*!< in: log file, or NULL */
    z_streamp strm, /*!< in/out: compressed stream for deflate() */
    int flush)      /*!< in: deflate() flushing method */
{
  int status;
  if (UNIV_UNLIKELY(page_zip_compress_dbg)) {
    ut_print_buf(stderr, strm->next_in, strm->avail_in);
  }
  if (UNIV_LIKELY_NULL(logfile)) {
    if (fwrite(strm->next_in, 1, strm->avail_in, logfile) != strm->avail_in) {
      perror("fwrite");
    }
  }
  status = deflate(strm, flush);
  if (UNIV_UNLIKELY(page_zip_compress_dbg)) {
    fprintf(stderr, " -> %d\n", status);
  }
  return (status);
}

#endif
