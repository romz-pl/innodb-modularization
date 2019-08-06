#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

#ifdef UNIV_DEBUG

/** Check if memo contains the given page.
@return	true if contains */
#define mtr_memo_contains_page(m, p, t) \
  (m)->memo_contains_page_flagged((p), (t))

#endif
