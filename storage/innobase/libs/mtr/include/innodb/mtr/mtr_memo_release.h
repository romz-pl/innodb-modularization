#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Release an object in the memo stack.
@return true if released */
#define mtr_memo_release(m, o, t) (m)->memo_release((o), (t))
