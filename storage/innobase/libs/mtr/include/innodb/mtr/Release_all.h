#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/memo_slot_release.h>
#include <innodb/mtr/mtr_memo_slot_t.h>

/** Release the latches and blocks acquired by the mini-transaction. */
struct Release_all {
  /** @return true always. */
  bool operator()(mtr_memo_slot_t *slot) const {
    if (slot->object != NULL) {
      memo_slot_release(slot);
    }

    return (true);
  }
};
