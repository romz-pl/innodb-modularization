#pragma once

#include <innodb/univ/univ.h>


#ifdef PAGE_ZIP_COMPRESS_DBG

/** Set this variable in a debugger to enable
binary logging of the data passed to deflate().
When this variable is nonzero, it will act
as a log file name generator. */
extern unsigned page_zip_compress_log;

#endif
