#pragma once

#include <innodb/univ/univ.h>
#include <innodb/disk/page_t.h>

/** Determine whether the page is empty.
 @return true if the page is empty (PAGE_N_RECS = 0) */
bool page_is_empty(const page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));
