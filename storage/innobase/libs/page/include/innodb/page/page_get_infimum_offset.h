#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Gets the offset of the first record on the page.
 @return offset of the first record in record list, relative from page */
ulint page_get_infimum_offset(
    const page_t *page); /*!< in: page which must have record(s) */
