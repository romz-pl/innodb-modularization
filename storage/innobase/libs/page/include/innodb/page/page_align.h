#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Gets the start of a page.
 @return start of the page */
page_t *page_align(const void *ptr) /*!< in: pointer to page frame */
    MY_ATTRIBUTE((const));
