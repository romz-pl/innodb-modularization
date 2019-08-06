#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

#ifdef UNIV_DEBUG

/** Check if memo contains the given item ignore if table is intrinsic
@return true if contains or table is intrinsic. */
#define mtr_is_block_fix(m, o, t, table) \
  (mtr_memo_contains(m, o, t) || table->is_intrinsic())


#endif
