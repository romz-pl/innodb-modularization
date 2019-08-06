#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Push an object to an mtr memo stack. */
#define mtr_memo_push(m, o, t) (m)->memo_push(o, t)
