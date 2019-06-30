#pragma once

#include <innodb/univ/univ.h>

/* Latch types; these are used also in btr0btr.h and mtr0mtr.h: keep the
numerical values smaller than 30 (smaller than BTR_MODIFY_TREE and
MTR_MEMO_MODIFY) and the order of the numerical values like below! and they
should be 2pow value to be used also as ORed combination of flag. */
enum rw_lock_type_t {
  RW_S_LATCH = 1,
  RW_X_LATCH = 2,
  RW_SX_LATCH = 4,
  RW_NO_LATCH = 8
};
