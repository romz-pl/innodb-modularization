#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

#define mtr_memo_contains_flagged(m, p, l) (m)->memo_contains_flagged((p), (l))
