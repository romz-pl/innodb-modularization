#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_t.h>

/** Determine whether a page is an index root page.
@param[in]	page	page frame
@return true if the page is a root page of an index */
bool page_is_root(const page_t *page) MY_ATTRIBUTE((warn_unused_result));
