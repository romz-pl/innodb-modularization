#pragma once

#include <innodb/univ/univ.h>


#ifdef PAGE_ZIP_COMPRESS_DBG

int page_zip_compress_deflate(
    FILE *logfile,  /*!< in: log file, or NULL */
    z_streamp strm, /*!< in/out: compressed stream for deflate() */
    int flush);      /*!< in: deflate() flushing method */

#endif
