#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_rec/RecID.h>
#include <innodb/lock_priv/lock_t.h>
#include <innodb/hash/hash_cell_t.h>
#include <innodb/lock_sys/lock_sys.h>
#include <innodb/hash/hash_calc_hash.h>
#include <innodb/hash/hash_get_nth_cell.h>

/** Iterate over record locks matching <space, page_no, heap_no> */
struct Lock_iter {
  /* First is the previous lock, and second is the current lock. */
  /** Gets the next record lock on a page.
  @param[in]	rec_id		The record ID
  @param[in]	lock		The current lock
  @return matching lock or nullptr if end of list */
  static lock_t *advance(const RecID &rec_id, lock_t *lock) {
    ut_ad(lock_mutex_own());
    ut_ad(lock->is_record_lock());

    while ((lock = static_cast<lock_t *>(lock->hash)) != nullptr) {
      ut_ad(lock->is_record_lock());

      if (rec_id.matches(lock)) {
        return (lock);
      }
    }

    ut_ad(lock == nullptr);
    return (nullptr);
  }

  /** Gets the first explicit lock request on a record.
  @param[in]	list		Record hash
  @param[in]	rec_id		Record ID
  @return	first lock, nullptr if none exists */
  static lock_t *first(hash_cell_t *list, const RecID &rec_id) {
    ut_ad(lock_mutex_own());

    auto lock = static_cast<lock_t *>(list->node);

    ut_ad(lock == nullptr || lock->is_record_lock());

    if (lock != nullptr && !rec_id.matches(lock)) {
      lock = advance(rec_id, lock);
    }

    return (lock);
  }

  /** Iterate over all the locks on a specific row
  @param[in]	rec_id		Iterate over locks on this row
  @param[in]	f		Function to call for each entry
  @return lock where the callback returned false */
  template <typename F>
  static const lock_t *for_each(const RecID &rec_id, F &&f) {
    ut_ad(lock_mutex_own());

    auto hash_table = lock_sys->rec_hash;

    auto list = hash_get_nth_cell(hash_table,
                                  hash_calc_hash(rec_id.m_fold, hash_table));

    for (auto lock = first(list, rec_id); lock != nullptr;
         lock = advance(rec_id, lock)) {
      ut_ad(lock->is_record_lock());

      if (!f(lock)) {
        return (lock);
      }
    }

    return (nullptr);
  }
};
