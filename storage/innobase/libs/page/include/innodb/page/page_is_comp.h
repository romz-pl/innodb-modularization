#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_t.h>

/** Determine whether the page is in new-style compact format.
 @return nonzero if the page is in compact format, zero if it is in
 old-style format */
ulint page_is_comp(const page_t *page); /*!< in: index page */
