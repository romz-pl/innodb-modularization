#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_memo_slot_t.h>
#include <innodb/assert/assert.h>

/** Check that all slots have been handled. */
struct Debug_check {
  /** @return true always. */
  bool operator()(const mtr_memo_slot_t *slot) const {
    ut_a(slot->object == NULL);
    return (true);
  }
};
