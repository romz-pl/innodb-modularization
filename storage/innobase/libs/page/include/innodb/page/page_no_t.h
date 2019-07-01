#pragma once

#include <innodb/univ/univ.h>

/** Page number */
typedef uint32_t page_no_t;


/** Get the predecessor of a file page.
@param[in]	page		File page
@return FIL_PAGE_PREV */
page_no_t fil_page_get_prev(const byte *page)
    MY_ATTRIBUTE((warn_unused_result));


/** Get the successor of a file page.
@param[in]	page		File page
@return FIL_PAGE_NEXT */
page_no_t fil_page_get_next(const byte *page)
    MY_ATTRIBUTE((warn_unused_result));
