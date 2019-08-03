#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Determine whether the page is a B-tree leaf.
 @return true if the page is a B-tree leaf (PAGE_LEVEL = 0) */
bool page_is_leaf(const page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));
