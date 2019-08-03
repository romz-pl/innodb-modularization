#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/disk/page_no_t.h>

/** Gets the page number.
 @return page number */
page_no_t page_get_page_no(const page_t *page); /*!< in: page */
