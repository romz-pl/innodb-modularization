#include <innodb/page/page_zip_fail.h>

#ifdef UNIV_ZIP_DEBUG

/** Wrapper for page_zip_fail_func()
@param fmt_args in: printf(3) format string and arguments */
#define page_zip_fail(fmt_args) page_zip_fail_func fmt_args

#endif
