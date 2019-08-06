#pragma once

#include <innodb/univ/univ.h>

/** Mini-transaction memo stack slot. */
struct mtr_memo_slot_t {
  /** pointer to the object */
  void *object;

  /** type of the stored object (MTR_MEMO_S_LOCK, ...) */
  ulint type;
};
