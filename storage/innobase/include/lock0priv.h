/*****************************************************************************

Copyright (c) 2007, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/lock0priv.h
 Lock module internal structures and methods.

 Created July 12, 2007 Vasil Dimov
 *******************************************************/

#ifndef lock0priv_h
#define lock0priv_h

#include <innodb/univ/univ.h>

#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/lock_types/lock_mode_string.h>
#include <innodb/lock_priv/lock_table_t.h>
#include <innodb/lock_priv/lock_rec_t.h>
#include <innodb/lock_priv/lock_t.h>
#include <innodb/lock_priv/lock_get_type_low.h>
#include <innodb/lock_priv/lock_rec_fold.h>
#include <innodb/lock_priv/lock_compatibility_matrix.h>
#include <innodb/lock_priv/lock_strength_matrix.h>
#include <innodb/lock_priv/lock_rec_req_status.h>
#include <innodb/record/RecID.h>
#include <innodb/lock_priv/lock_rec_get_prev.h>
#include <innodb/lock_priv/lock_cancel_waiting_and_release.h>
#include <innodb/lock_priv/lock_reset_wait_and_release_thread_if_suspended.h>
#include <innodb/lock_priv/lock_clust_rec_some_has_impl.h>
#include <innodb/lock_priv/lock_rec_get_next_on_page_const.h>
#include <innodb/lock_priv/lock_rec_get_nth_bit.h>
#include <innodb/lock_priv/lock_rec_set_nth_bit.h>
#include <innodb/lock_priv/lock_rec_get_next_on_page.h>
#include <innodb/lock_priv/lock_rec_get_first_on_page_addr.h>
#include <innodb/lock_priv/lock_rec_get_first_on_page.h>
#include <innodb/lock_priv/lock_rec_get_next.h>
#include <innodb/lock_priv/lock_rec_get_next_const.h>
#include <innodb/lock_priv/lock_rec_get_first.h>
#include <innodb/lock_priv/lock_get_mode.h>
#include <innodb/lock_priv/lock_mode_compatible.h>
#include <innodb/lock_priv/lock_mode_stronger_or_eq.h>
#include <innodb/lock_priv/lock_get_wait.h>
#include <innodb/lock_priv/lock_rec_find_similar_on_page.h>
#include <innodb/lock_priv/lock_table_has.h>


#ifndef LOCK_MODULE_IMPLEMENTATION
/* If you need to access members of the structures defined in this
file, please write appropriate functions that retrieve them and put
those functions in lock/ */
#error Do not include lock0priv.h outside of the lock/ module
#endif


#include <utility>








inline
hash_table_t *lock_t::hash_table() const { return (lock_hash_get(type_mode)); }

inline
trx_que_t lock_t::trx_que_state() const { return (trx->lock.que_state); }











#ifdef UNIV_DEBUG
extern ibool lock_print_waits;
#endif /* UNIV_DEBUG */










/**
Create record locks */
class RecLock {
 public:
  /**
  @param[in,out] thr	Transaction query thread requesting the record
                          lock
  @param[in] index	Index on which record lock requested
  @param[in] rec_id	Record lock tuple {space, page_no, heap_no}
  @param[in] mode		The lock mode */
  RecLock(que_thr_t *thr, dict_index_t *index, const RecID &rec_id, ulint mode)
      : m_thr(thr),
        m_trx(thr_get_trx(thr)),
        m_mode(mode),
        m_index(index),
        m_rec_id(rec_id) {
    ut_ad(is_predicate_lock(m_mode));

    init(NULL);
  }

  /**
  @param[in,out] thr	Transaction query thread requesting the record
                          lock
  @param[in] index	Index on which record lock requested
  @param[in] block	Buffer page containing record
  @param[in] heap_no	Heap number within the block
  @param[in] mode		The lock mode
  @param[in] prdt		The predicate for the rtree lock */
  RecLock(que_thr_t *thr, dict_index_t *index, const buf_block_t *block,
          ulint heap_no, ulint mode, lock_prdt_t *prdt = NULL)
      : m_thr(thr),
        m_trx(thr_get_trx(thr)),
        m_mode(mode),
        m_index(index),
        m_rec_id(block, heap_no) {
    btr_assert_not_corrupted(block, index);

    init(block->frame);
  }

  /**
  @param[in] index	Index on which record lock requested
  @param[in] rec_id	Record lock tuple {space, page_no, heap_no}
  @param[in] mode		The lock mode */
  RecLock(dict_index_t *index, const RecID &rec_id, ulint mode)
      : m_thr(), m_trx(), m_mode(mode), m_index(index), m_rec_id(rec_id) {
    ut_ad(is_predicate_lock(m_mode));

    init(NULL);
  }

  /**
  @param[in] index	Index on which record lock requested
  @param[in] block	Buffer page containing record
  @param[in] heap_no	Heap number withing block
  @param[in] mode		The lock mode */
  RecLock(dict_index_t *index, const buf_block_t *block, ulint heap_no,
          ulint mode)
      : m_thr(),
        m_trx(),
        m_mode(mode),
        m_index(index),
        m_rec_id(block, heap_no) {
    btr_assert_not_corrupted(block, index);

    init(block->frame);
  }

  /**
  Enqueue a lock wait for a transaction. If it is a high priority
  transaction (cannot rollback) then jump ahead in the record lock wait
  queue and if the transaction at the head of the queue is itself waiting
  roll it back.
  @param[in, out] wait_for	The lock that the the joining
                                  transaction is waiting for
  @param[in] prdt			Predicate [optional]
  @return DB_LOCK_WAIT, DB_DEADLOCK, or
          DB_SUCCESS_LOCKED_REC; DB_SUCCESS_LOCKED_REC means that
          there was a deadlock, but another transaction was chosen
          as a victim, and we got the lock immediately: no need to
          wait then */
  dberr_t add_to_waitq(const lock_t *wait_for, const lock_prdt_t *prdt = NULL);

  /**
  Create a lock for a transaction and initialise it.
  @param[in, out] trx		Transaction requesting the new lock
  @param[in] add_to_hash		add the lock to hash table
  @param[in] prdt			Predicate lock (optional)
  @return new lock instance */
  lock_t *create(trx_t *trx, bool add_to_hash,
                 const lock_prdt_t *prdt = nullptr);

  /**
  Check of the lock is on m_rec_id.
  @param[in] lock			Lock to compare with
  @return true if the record lock is on m_rec_id*/
  bool is_on_row(const lock_t *lock) const;

  /**
  Create the lock instance
  @param[in, out] trx	The transaction requesting the lock
  @param[in, out] index	Index on which record lock is required
  @param[in] mode		The lock mode desired
  @param[in] rec_id	The record id
  @param[in] size		Size of the lock + bitmap requested
  @return a record lock instance */
  static lock_t *lock_alloc(trx_t *trx, dict_index_t *index, ulint mode,
                            const RecID &rec_id, ulint size);

 private:
  /*
  @return the record lock size in bytes */
  size_t lock_size() const { return (m_size); }

  /**
  Do some checks and prepare for creating a new record lock */
  void prepare() const;

  /**
  Collect the transactions that will need to be rolled back asynchronously
  @param[in, out] trx	Transaction to be rolled back */
  void mark_trx_for_rollback(trx_t *trx);

  /**
  Jump the queue for the record over all low priority transactions and
  add the lock. If all current granted locks are compatible, grant the
  lock. Otherwise, mark all granted transaction for asynchronous
  rollback and add to hit list.
  @param[in, out]	lock		Lock being requested
  @param[in]	conflict_lock	First conflicting lock from the head
  @return true if the lock is granted */
  bool jump_queue(lock_t *lock, const lock_t *conflict_lock);

  /** Find position in lock queue and add the high priority transaction
  lock. Intention and GAP only locks can be granted even if there are
  waiting locks in front of the queue. To add the High priority
  transaction in a safe position we keep the following rule.

  1. If the lock can be granted, add it before the first waiting lock
  in the queue so that all currently waiting locks need to do conflict
  check before getting granted.

  2. If the lock has to wait, add it after the last granted lock or the
  last waiting high priority transaction in the queue whichever is later.
  This ensures that the transaction is granted only after doing conflict
  check with all granted transactions.
  @param[in]      lock            Lock being requested
  @param[in]      conflict_lock   First conflicting lock from the head
  @param[out]     high_priority   high priority transaction ahead in queue
  @return true if the lock can be granted */
  bool lock_add_priority(lock_t *lock, const lock_t *conflict_lock,
                         bool *high_priority);

  /** Iterate over the granted locks and prepare the hit list for
  ASYNC Rollback.

  If the transaction is waiting for some other lock then wake up
  with deadlock error.  Currently we don't mark following transactions
  for ASYNC Rollback.

  1. Read only transactions
  2. Background transactions
  3. Other High priority transactions
  @param[in]      lock            Lock being requested
  @param[in]      conflict_lock   First conflicting lock from the head */
  void make_trx_hit_list(lock_t *lock, const lock_t *conflict_lock);

  /**
  Setup the requesting transaction state for lock grant
  @param[in,out] lock	Lock for which to change state */
  void set_wait_state(lock_t *lock);

  /**
  Add the lock to the record lock hash and the transaction's lock list
  @param[in,out] lock	Newly created record lock to add to the
                          rec hash and the transaction lock list
  @param[in] add_to_hash	If the lock should be added to the hash table */
  void lock_add(lock_t *lock, bool add_to_hash);

  /**
  Check and resolve any deadlocks
  @param[in, out] lock		The lock being acquired
  @return DB_LOCK_WAIT, DB_DEADLOCK, or
          DB_SUCCESS_LOCKED_REC; DB_SUCCESS_LOCKED_REC means that
          there was a deadlock, but another transaction was chosen
          as a victim, and we got the lock immediately: no need to
          wait then */
  dberr_t deadlock_check(lock_t *lock);

  /**
  Check the outcome of the deadlock check
  @param[in,out] victim_trx	Transaction selected for rollback
  @param[in,out] lock		Lock being requested
  @return DB_LOCK_WAIT, DB_DEADLOCK or DB_SUCCESS_LOCKED_REC */
  dberr_t check_deadlock_result(const trx_t *victim_trx, lock_t *lock);

  /**
  Setup the context from the requirements */
  void init(const page_t *page) {
    ut_ad(lock_mutex_own());
    ut_ad(!srv_read_only_mode);
    ut_ad(m_index->is_clustered() || !dict_index_is_online_ddl(m_index));
    ut_ad(m_thr == NULL || m_trx == thr_get_trx(m_thr));

    m_size = is_predicate_lock(m_mode) ? lock_size(m_mode) : lock_size(page);

    /** If rec is the supremum record, then we reset the
    gap and LOCK_REC_NOT_GAP bits, as all locks on the
    supremum are automatically of the gap type */

    if (m_rec_id.m_heap_no == PAGE_HEAP_NO_SUPREMUM) {
      ut_ad(!(m_mode & LOCK_REC_NOT_GAP));

      m_mode &= ~(LOCK_GAP | LOCK_REC_NOT_GAP);
    }
  }

  /**
  Calculate the record lock physical size required for a predicate lock.
  @param[in] mode For predicate locks the lock mode
  @return the size of the lock data structure required in bytes */
  static size_t lock_size(ulint mode) {
    ut_ad(is_predicate_lock(mode));

    /* The lock is always on PAGE_HEAP_NO_INFIMUM(0),
    so we only need 1 bit (which is rounded up to 1
    byte) for lock bit setting */

    size_t n_bytes;

    if (mode & LOCK_PREDICATE) {
      const ulint align = UNIV_WORD_SIZE - 1;

      /* We will attach the predicate structure
      after lock. Make sure the memory is
      aligned on 8 bytes, the mem_heap_alloc
      will align it with MEM_SPACE_NEEDED
      anyway. */

      n_bytes = (1 + sizeof(lock_prdt_t) + align) & ~align;

      /* This should hold now */

      ut_ad(n_bytes == sizeof(lock_prdt_t) + UNIV_WORD_SIZE);

    } else {
      n_bytes = 1;
    }

    return (n_bytes);
  }

  /**
  Calculate the record lock physical size required, non-predicate lock.
  @param[in] page		For non-predicate locks the buffer page
  @return the size of the lock data structure required in bytes */
  static size_t lock_size(const page_t *page) {
    ulint n_recs = page_dir_get_n_heap(page);

    /* Make lock bitmap bigger by a safety margin */

    return (1 + ((n_recs + LOCK_PAGE_BITMAP_MARGIN) / 8));
  }

  /**
  @return true if the requested lock mode is for a predicate
          or page lock */
  static bool is_predicate_lock(ulint mode) {
    return (mode & (LOCK_PREDICATE | LOCK_PRDT_PAGE));
  }

 private:
  /** The query thread of the transaction */
  que_thr_t *m_thr;

  /**
  Transaction requesting the record lock */
  trx_t *m_trx;

  /**
  Lock mode requested */
  ulint m_mode;

  /**
  Size of the record lock in bytes */
  size_t m_size;

  /**
  Index on which the record lock is required */
  dict_index_t *m_index;

  /**
  The record lock tuple {space, page_no, heap_no} */
  RecID m_rec_id;
};

#ifdef UNIV_DEBUG
/** The count of the types of locks. */
static const ulint lock_types = UT_ARR_SIZE(lock_compatibility_matrix);
#endif /* UNIV_DEBUG */







#include "lock0priv.ic"

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

#endif /* lock0priv_h */
