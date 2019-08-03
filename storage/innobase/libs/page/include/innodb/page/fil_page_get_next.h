#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_no_t.h>

/** Get the successor of a file page.
@param[in]	page		File page
@return FIL_PAGE_NEXT */
page_no_t fil_page_get_next(const byte *page)
    MY_ATTRIBUTE((warn_unused_result));
