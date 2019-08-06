#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

#define mtr_block_sx_latch_at_savepoint(m, s, b) \
  (m)->sx_latch_at_savepoint((s), (b))
