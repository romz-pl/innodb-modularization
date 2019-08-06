#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

#define mtr_block_x_latch_at_savepoint(m, s, b) \
  (m)->x_latch_at_savepoint((s), (b))
