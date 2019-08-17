#pragma once

#include <innodb/univ/univ.h>



#if defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG

int page_zip_fail_func(const char *fmt, /*!< in: printf(3) format string */
                       ...); /*!< in: arguments corresponding to fmt */

#else


/** Dummy wrapper for page_zip_fail_func()
@param fmt_args ignored: printf(3) format string and arguments */
#define page_zip_fail(fmt_args) /* empty */

#endif
