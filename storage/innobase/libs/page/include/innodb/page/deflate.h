#pragma once

#include <innodb/univ/univ.h>


#ifdef PAGE_ZIP_COMPRESS_DBG

/* Redefine deflate(). */
#undef deflate

/** Debug wrapper for the zlib compression routine deflate().
Log the operation if page_zip_compress_dbg is set.
@param strm in/out: compressed stream
@param flush in: flushing method
@return deflate() status: Z_OK, Z_BUF_ERROR, ... */

#define deflate(strm, flush) page_zip_compress_deflate(logfile, strm, flush)

#endif
