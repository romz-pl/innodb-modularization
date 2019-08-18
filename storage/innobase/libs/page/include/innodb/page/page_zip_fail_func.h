#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_ZIP_DEBUG

int page_zip_fail_func(const char *fmt, ...);

#endif /* UNIV_ZIP_DEBUG */
