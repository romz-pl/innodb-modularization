#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_memo_slot_t.h>
#include <innodb/assert/assert.h>

/** Find specific object */
struct Find {
  /** Constructor */
  Find(const void *object, ulint type)
      : m_slot(), m_type(type), m_object(object) {
    ut_a(object != NULL);
  }

  /** @return false if the object was found. */
  bool operator()(mtr_memo_slot_t *slot) {
    if (m_object == slot->object && m_type == slot->type) {
      m_slot = slot;
      return (false);
    }

    return (true);
  }

  /** Slot if found */
  mtr_memo_slot_t *m_slot;

  /** Type of the object to look for */
  ulint m_type;

  /** The object instance to look for */
  const void *m_object;
};
