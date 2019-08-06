#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_buf_t.h>
#include <innodb/mtr/mtr_memo_slot_t.h>

/** Iterate over a memo block in reverse. */
template <typename Functor>
struct Iterate {
  /** Release specific object */
  explicit Iterate(Functor &functor) : m_functor(functor) { /* Do nothing */
  }

  /** @return false if the functor returns false. */
  bool operator()(mtr_buf_t::block_t *block) {
    const mtr_memo_slot_t *start =
        reinterpret_cast<const mtr_memo_slot_t *>(block->begin());

    mtr_memo_slot_t *slot = reinterpret_cast<mtr_memo_slot_t *>(block->end());

    ut_ad(!(block->used() % sizeof(*slot)));

    while (slot-- != start) {
      if (!m_functor(slot)) {
        return (false);
      }
    }

    return (true);
  }

  Functor &m_functor;
};
