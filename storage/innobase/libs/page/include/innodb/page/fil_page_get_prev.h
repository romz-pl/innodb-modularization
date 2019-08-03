#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_no_t.h>

/** Get the predecessor of a file page.
@param[in]	page		File page
@return FIL_PAGE_PREV */
page_no_t fil_page_get_prev(const byte *page)
    MY_ATTRIBUTE((warn_unused_result));
