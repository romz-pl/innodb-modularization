#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Gets the offset of the last record on the page.
 @return offset of the last record in record list, relative from page */
ulint page_get_supremum_offset(
    const page_t *page); /*!< in: page which must have record(s) */
