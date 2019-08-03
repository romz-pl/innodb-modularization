#pragma once

#include <innodb/univ/univ.h>

/** Gets the offset within a page.
 @return offset from the start of the page */
ulint page_offset(const void *ptr) /*!< in: pointer to page frame */
    MY_ATTRIBUTE((const));
