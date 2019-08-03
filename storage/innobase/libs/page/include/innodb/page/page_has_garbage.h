#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_t.h>

/** Determine whether the page contains garbage.
 @return true if the page contains garbage (PAGE_GARBAGE is not 0) */
bool page_has_garbage(const page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));
