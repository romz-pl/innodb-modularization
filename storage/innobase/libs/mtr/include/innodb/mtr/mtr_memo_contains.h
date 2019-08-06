#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

#ifdef UNIV_DEBUG

/** Check if memo contains the given item.
@return	true if contains */
#define mtr_memo_contains(m, o, t) (m)->memo_contains((m)->get_memo(), (o), (t))


#endif
