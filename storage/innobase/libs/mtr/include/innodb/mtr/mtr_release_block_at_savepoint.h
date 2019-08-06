#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

#define mtr_release_block_at_savepoint(m, s, b) \
  (m)->release_block_at_savepoint((s), (b))
