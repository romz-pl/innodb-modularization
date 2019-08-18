#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_ZIP_DEBUG

/** Flag: make page_zip_validate() compare page headers only */
extern ibool page_zip_validate_header_only;

#endif
