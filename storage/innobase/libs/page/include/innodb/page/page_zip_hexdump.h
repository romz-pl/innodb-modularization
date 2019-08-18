#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_ZIP_DEBUG

/** Dump a block of memory on the standard error stream.
@param buf in: data
@param size in: length of the data, in bytes */
#define page_zip_hexdump(buf, size) page_zip_hexdump_func(#buf, buf, size)

#endif
