#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

#ifdef UNIV_DEBUG

/** Check if memo contains the given page ignore if table is intrinsic
@return true if contains or table is intrinsic. */
#define mtr_is_page_fix(m, p, t, table) \
  (mtr_memo_contains_page(m, p, t) || table->is_intrinsic())

#endif
