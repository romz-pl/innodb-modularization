#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/ut_error.h>
#include <innodb/hash/hash_table_t.h>
#include <innodb/lock_priv/flags.h>
#include <innodb/lock_priv/lock_rec_t.h>
#include <innodb/lock_priv/lock_table_t.h>
#include <innodb/lock_types/lock_mode.h>
#include <innodb/lst/lst.h>
#include <innodb/trx_types/trx_que_t.h>
#include <innodb/univ/page_no_t.h>


struct dict_index_t;
struct trx_t;

/** Lock struct; protected by lock_sys->mutex */
struct lock_t {
  /** transaction owning the lock */
  trx_t *trx;

  /** list of the locks of the transaction */
  UT_LIST_NODE_T(lock_t) trx_locks;

  /** Index for a record lock */
  dict_index_t *index;

  /** Hash chain node for a record lock. The link node in a singly
  linked list, used by the hash table. */
  lock_t *hash;

  union {
    /** Table lock */
    lock_table_t tab_lock;

    /** Record lock */
    lock_rec_t rec_lock;
  };

#ifdef HAVE_PSI_THREAD_INTERFACE
#ifdef HAVE_PSI_DATA_LOCK_INTERFACE
  /** Performance schema thread that created the lock. */
  ulonglong m_psi_internal_thread_id;

  /** Performance schema event that created the lock. */
  ulonglong m_psi_event_id;
#endif /* HAVE_PSI_DATA_LOCK_INTERFACE */
#endif /* HAVE_PSI_THREAD_INTERFACE */

  /** The lock type and mode bit flags.
  LOCK_GAP or LOCK_REC_NOT_GAP, LOCK_INSERT_INTENTION, wait flag, ORed */
  uint32_t type_mode;

#if defined(UNIV_DEBUG)
  /** Timestamp when it was created. */
  uint64_t m_seq;
#endif /* UNIV_DEBUG */

  /** Remove GAP lock from a next Key Lock */
  void remove_gap_lock() {
    ut_ad(!is_gap());
    ut_ad(!is_insert_intention());
    ut_ad(is_record_lock());

    type_mode |= LOCK_REC_NOT_GAP;
  }

  /** Determine if the lock object is a record lock.
  @return true if record lock, false otherwise. */
  bool is_record_lock() const { return (type() == LOCK_REC); }

  /** Determine if it is predicate lock.
  @return true if predicate lock, false otherwise. */
  bool is_predicate() const {
    return (type_mode & (LOCK_PREDICATE | LOCK_PRDT_PAGE));
  }

  /** @return true if the lock wait flag is set */
  bool is_waiting() const { return (type_mode & LOCK_WAIT); }

  /** @return true if the gap lock bit is set */
  bool is_gap() const { return (type_mode & LOCK_GAP); }

  /** @return true if the not gap lock bit is set */
  bool is_record_not_gap() const { return (type_mode & LOCK_REC_NOT_GAP); }

  /** @return true if the insert intention bit is set */
  bool is_insert_intention() const {
    return (type_mode & LOCK_INSERT_INTENTION);
  }

  /** @return the lock mode */
  uint32_t type() const { return (type_mode & LOCK_TYPE_MASK); }

  /** @return the precise lock mode */
  lock_mode mode() const {
    return (static_cast<lock_mode>(type_mode & LOCK_MODE_MASK));
  }

  /** Get lock hash table
  @return lock hash table */
  hash_table_t *hash_table() const;

  /** @return the record lock tablespace ID */
  space_id_t space_id() const {
    ut_ad(is_record_lock());

    return (rec_lock.space);
  }

  /** @return the record lock page number */
  page_no_t page_no() const {
    ut_ad(is_record_lock());

    return (rec_lock.page_no);
  }

  /** @return the transaction's query thread state. */
  trx_que_t trx_que_state() const;

  /** Print the lock object into the given output stream.
  @param[in,out]	out	the output stream
  @return the given output stream. */
  std::ostream &print(std::ostream &out) const;

  /** Convert the member 'type_mode' into a human readable string.
  @return human readable string */
  std::string type_mode_string() const;

  /* @return the string/text representation of the record type. */
  const char *type_string() const {
    switch (type_mode & LOCK_TYPE_MASK) {
      case LOCK_REC:
        return ("LOCK_REC");
      case LOCK_TABLE:
        return ("LOCK_TABLE");
      default:
        ut_error;
    }
  }
};
