#pragma once

#include <innodb/univ/univ.h>

#include <innodb/assert/assert.h>
#include <innodb/sync_rw/rw_lock_s_lock.h>
#include <innodb/sync_rw/rw_lock_s_unlock.h>
#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>
#include <innodb/trx_types/Rseg_Iterator.h>
#include <innodb/trx_types/trx_rseg_t.h>

//
// In file
//     /usr/include/bits/utmpx.h
// is defined macro
//     #define EMPTY 0
// This macro conflicts with values of enumeration "undo_space_states"
// defined in class Rsegs
//
#if defined(EMPTY)
#undef EMPTY
#endif

/** This is a wrapper for a std::vector of trx_rseg_t object pointers. */
class Rsegs {
 public:
  /** Default constructor */
  Rsegs() : m_rsegs(), m_latch(), m_state(INIT) {
#ifndef UNIV_HOTBACKUP
    init();
#endif /* !UNIV_HOTBACKUP */
  }

  ~Rsegs() {
#ifndef UNIV_HOTBACKUP
    deinit();
#endif /* !UNIV_HOTBACKUP */
  }

  /** Initialize */
  void init();

  /** De-initialize */
  void deinit();

  /** Clear the vector of cached rollback segments leaving the
  reserved space allocated. */
  void clear();

  /** Add rollback segment.
  @param[in]	rseg	rollback segment to add. */
  void push_back(trx_rseg_t *rseg) { m_rsegs.push_back(rseg); }

  /** Number of registered rsegs.
  @return size of rseg list. */
  ulint size() { return (m_rsegs.size()); }

  /** beginning iterator
  @return an iterator to the first element */
  Rseg_Iterator begin() { return (m_rsegs.begin()); }

  /** ending iterator
  @return an iterator to the end */
  Rseg_Iterator end() { return (m_rsegs.end()); }

  /** Find the rseg at the given slot in this vector.
  @param[in]	slot	a slot within the vector.
  @return an iterator to the end */
  trx_rseg_t *at(ulint slot) { return (m_rsegs.at(slot)); }

  /** Find an rseg in the std::vector that uses the rseg_id given.
  @param[in]	rseg_id		A slot in a durable array such as
                                  the TRX_SYS page or RSEG_ARRAY page.
  @return a pointer to an trx_rseg_t that uses the rseg_id. */
  trx_rseg_t *find(ulint rseg_id);

  /** Sort the vector on trx_rseg_t::id */
  void sort() {
    if (m_rsegs.empty()) {
      return;
    }

    std::sort(
        m_rsegs.begin(), m_rsegs.end(),
        [](trx_rseg_t *lhs, trx_rseg_t *rhs) { return (rhs->id > lhs->id); });
  }

  /** Get a shared lock on m_rsegs. */
  void s_lock() { rw_lock_s_lock(m_latch); }

  /** Get a shared lock on m_rsegs. */
  void s_unlock() { rw_lock_s_unlock(m_latch); }

  /** Get a shared lock on m_rsegs. */
  void x_lock() { rw_lock_x_lock(m_latch); }

  /** Get a shared lock on m_rsegs. */
  void x_unlock() { rw_lock_x_unlock(m_latch); }

  /** Return whether the undo tablespace is active.
  @return true if active */
  bool is_active() { return (m_state == ACTIVE); }

  /** Return whether the undo tablespace is inactive due to
  implicit selection by the purge thread.
  @return true if marked for truncation by the purge thread */
  bool is_inactive_implicit() { return (m_state == INACTIVE_IMPLICIT); }

  /** Return whether the undo tablespace was made inactive by
  ALTER TABLESPACE.
  @return true if altered */
  bool is_inactive_explicit() { return (m_state == INACTIVE_EXPLICIT); }

  /** Return whether the undo tablespace is empty and ready
  to be dropped.
  @return true if empty */
  bool is_empty() { return (m_state == EMPTY); }

  /** Return whether the undo tablespace is being initialized.
  @return true if empty */
  bool is_init() { return (m_state == INIT); }

  /** Set the state of the rollback segments in this undo tablespace
  to ACTIVE for use by new transactions. */
  void set_active() { m_state = ACTIVE; }

  /** Set the state of the rollback segments in this undo
  tablespace to inactive_implicit. This means that it will be
  truncated and then made active again by the purge thread.
  It will not be used for new transactions until it becomes
  active again. */
  void set_inactive_implicit() {
    ut_ad(m_state == ACTIVE || m_state == INACTIVE_EXPLICIT);
    m_state = INACTIVE_IMPLICIT;
  }

  /** Make the undo tablespace inactive so that it will not be
  used for new transactions.  The purge thread will clear out
  all the undo logs, truncate it, and then mark it empty. */
  void set_inactive_explicit() {
    ut_ad(m_state == ACTIVE || m_state == INACTIVE_IMPLICIT);
    m_state = INACTIVE_EXPLICIT;
  }

  /** Set the state of the undo tablespace to empty so that it
  can be dropped. */
  void set_empty() {
    ut_ad(m_state == INACTIVE_EXPLICIT || m_state == ACTIVE ||
          m_state == INIT || m_state == EMPTY);
    m_state = EMPTY;
  }

  /** std::vector of rollback segments */
  Rsegs_Vector m_rsegs;

 private:
  /** RW lock to protect m_rsegs vector, m_active, and each
  trx_rseg_t::trx_ref_count within it.
  m_rsegs:   x for adding elements, s for scanning, size etc.
  m_active:  x for modification, s for read
  each trx_rseg_t::trx_ref_count within m_rsegs
             s and atomic increment for modification, x for read */
  rw_lock_t *m_latch;

  /* The four states of an undo tablespace.
  INIT:     The initial state of an undo space that is being created or opened.
  ACTIVE:   The rollback segments in this tablespace can be allocated to new
            transactions.  The undo tablespace is ready for undo logs.
  INACTIVE_IMPLICIT: These rollback segments are no longer being used by new
            transactions.  They arre 'inactive'. The truncate process
            is happening. This undo tablespace was selected by the
            purge thread implicitly. When the truncation process
            is complete, the next state is ACTIVE.
  INACTIVE_EXPLICIT:  These rollback segments are no longer being used by new
            transactions.  They arre 'inactive'. The truncate process
            is happening. This undo tablespace was selected by the
            an ALTER UNDO TABLESPACE  SET INACTIVE command. When the
            truncation process is complete, the next state is EMPTY.
  EMPTY:    The undo tablespace has been truncated but is no longer
            active. It is ready to be either dropped or set active
            explicitly. This state is also used when the undo tablespace and
            its rollback segments are being inititalized.

  These states are changed under an exclusive lock on m_latch and are read
  under a shared lock.

  The following actions can cause changes in these states:
  Init:         Implicit undo spaces are created at startup.
  Create:       Explicit undo tablespace creation at runtime.
  Mark:         Purge thread implicitly selects an undo space to truncate.
  SetInactive:  This ALTER UNDO TABLESPACE causes an explicit truncation.
  SetActive:    This ALTER UNDO TABLESPACE changes the target state from
                EMPTY to ACTIVE.
  Trucate:      The truncate process is completed by the purge thread.
  Drop:         Delete an EMPTY undo tablespace
  Crash:        A crash occurs
  Fixup:        At startup, if an undo space was being truncated with a crash.
  SaveDDState:  At startup, once the DD is available the state saved there
                will be applied.  INACTIVE_IMPLICIT is never saved to the DD.
                So the DD state INACTIVE means INACTIVE_EXPLICIT.
                See apply_dd_undo_state()

  State changes allowed: (Actions on states not mentioned are not allowed.)
  Init         from null -> INIT -> ACTIVE see srv_start()
               from null -> INIT -> EMPTY  see trx_rsegs_init()
  Create       from null -> EMPTY -> ACTIVE
  Mark         from INACTIVE_EXPLICIT -> INACTIVE_EXPLICIT -> Truncate
               from ACTIVE -> INACTIVE_IMPLICIT -> Truncate
  SetInactive  from ACTIVE -> INACTIVE_EXPLICIT -> Mark
               from INACTIVE_IMPLICIT -> INACTIVE_EXPLICIT
               from INACTIVE_EXPLICIT -> INACTIVE_EXPLICIT
               from EMPTY -> EMPTY
  SetActive    from ACTIVE -> ACTIVE
               from INACTIVE_IMPLICIT -> INACTIVE_IMPLICIT
               from INACTIVE_EXPLICIT -> INACTIVE_IMPLICIT
               from EMPTY -> ACTIVE
  Truncate     from INACTIVE_IMPLICIT -> ACTIVE
               from INACTIVE_EXPLICIT -> EMPTY
  Drop         if ACTIVE -> error returned
               if INACTIVE_IMPLICIT -> error returned
               if INACTIVE_EXPLICIT -> error returned
               from EMPTY -> null
  Crash        if ACTIVE, at startup:  ACTIVE
               if INACTIVE_IMPLICIT, at startup: Fixup
               if INACTIVE_EXPLICIT, at startup: Fixup
               if EMPTY, at startup:  EMPTY
  Fixup        from INACTIVE_IMPLICIT before crash -> INACTIVE_IMPLICIT -> Mark
               from INACTIVE_EXPLICIT before crash -> INACTIVE_IMPLICIT -> Mark
  SaveDDState  from ACTIVE before crash -> ACTIVE
               from INACTIVE_IMPLICIT before crash -> ACTIVE
               from INACTIVE_EXPLICIT before crash -> INACTIVE_EXPLICIT -> Mark
               from EMPTY -> EMPTY
  */
  enum undo_space_states {
    INIT,
    ACTIVE,
    INACTIVE_IMPLICIT,
    INACTIVE_EXPLICIT,
    EMPTY
  };

  /** The current state of this undo tablespace. */
  enum undo_space_states m_state;
};
