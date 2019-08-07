#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_buf_t.h>
#include <innodb/mtr/mtr_memo_type_t.h>
#include <innodb/mtr/mtr_log_t.h>
#include <innodb/mtr/mtr_state_t.h>
#include <innodb/log_redo/mlog_id_t.h>

#include <innodb/buffer/buf_block_t.h>

class FlushObserver;
struct fil_space_t;

/** Mini-transaction handle and buffer */
struct mtr_t {
  /** State variables of the mtr */
  struct Impl {
    /** memo stack for locks etc. */
    mtr_buf_t m_memo;

    /** mini-transaction log */
    mtr_buf_t m_log;

    /** true if mtr has made at least one buffer pool page dirty */
    bool m_made_dirty;

    /** true if inside ibuf changes */
    bool m_inside_ibuf;

    /** true if the mini-transaction modified buffer pool pages */
    bool m_modifications;

    /** Count of how many page initial log records have been
    written to the mtr log */
    ib_uint32_t m_n_log_recs;

    /** specifies which operations should be logged; default
    value MTR_LOG_ALL */
    mtr_log_t m_log_mode;

    /** State of the transaction */
    mtr_state_t m_state;

    /** Flush Observer */
    FlushObserver *m_flush_observer;

#ifdef UNIV_DEBUG
    /** For checking corruption. */
    ulint m_magic_n;
#endif /* UNIV_DEBUG */

    /** Owning mini-transaction */
    mtr_t *m_mtr;
  };

  mtr_t() { m_impl.m_state = MTR_STATE_INIT; }

  ~mtr_t() {
#ifdef UNIV_DEBUG
    switch (m_impl.m_state) {
      case MTR_STATE_ACTIVE:
        ut_ad(m_impl.m_memo.size() == 0);
        break;
      case MTR_STATE_INIT:
      case MTR_STATE_COMMITTED:
        break;
      case MTR_STATE_COMMITTING:
        ut_error;
    }
#endif /* UNIV_DEBUG */
  }

  /** Start a mini-transaction.
  @param sync		true if it is a synchronous mini-transaction
  @param read_only	true if read only mini-transaction */
  void start(bool sync = true, bool read_only = false);

  /** @return whether this is an asynchronous mini-transaction. */
  bool is_async() const { return (!m_sync); }

  /** Request a future commit to be synchronous. */
  void set_sync() { m_sync = true; }

  /** Commit the mini-transaction. */
  void commit();

  /** Return current size of the buffer.
  @return	savepoint */
  ulint get_savepoint() const MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(is_active());
    ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);

    return (m_impl.m_memo.size());
  }

  /** Release the (index tree) s-latch stored in an mtr memo after a
  savepoint.
  @param savepoint	value returned by @see set_savepoint.
  @param lock		latch to release */
  void release_s_latch_at_savepoint(ulint savepoint, rw_lock_t *lock);

  /** Release the block in an mtr memo after a savepoint. */
  void release_block_at_savepoint(ulint savepoint, buf_block_t *block);

  /** SX-latch a not yet latched block after a savepoint. */
  void sx_latch_at_savepoint(ulint savepoint, buf_block_t *block);

  /** X-latch a not yet latched block after a savepoint. */
  void x_latch_at_savepoint(ulint savepoint, buf_block_t *block);

  /** Get the logging mode.
  @return	logging mode */
  mtr_log_t get_log_mode() const MY_ATTRIBUTE((warn_unused_result));

  /** Change the logging mode.
  @param mode	 logging mode
  @return	old mode */
  mtr_log_t set_log_mode(mtr_log_t mode);

  /** Read 1 - 4 bytes from a file page buffered in the buffer pool.
  @param ptr	pointer from where to read
  @param type	MLOG_1BYTE, MLOG_2BYTES, MLOG_4BYTES
  @return	value read */
  uint32_t read_ulint(const byte *ptr, mlog_id_t type) const
      MY_ATTRIBUTE((warn_unused_result));

  /** Locks a rw-latch in S mode.
  NOTE: use mtr_s_lock().
  @param lock	rw-lock
  @param file	file name from where called
  @param line	line number in file */
  void s_lock(rw_lock_t *lock, const char *file, ulint line);

  /** Locks a rw-latch in X mode.
  NOTE: use mtr_x_lock().
  @param lock	rw-lock
  @param file	file name from where called
  @param line	line number in file */
  void x_lock(rw_lock_t *lock, const char *file, ulint line);

  /** Locks a rw-latch in X mode.
  NOTE: use mtr_sx_lock().
  @param lock	rw-lock
  @param file	file name from where called
  @param line	line number in file */
  void sx_lock(rw_lock_t *lock, const char *file, ulint line);

  /** Acquire a tablespace X-latch.
  NOTE: use mtr_x_lock_space().
  @param[in]	space		tablespace instance
  @param[in]	file		file name from where called
  @param[in]	line		line number in file */
  void x_lock_space(fil_space_t *space, const char *file, ulint line);

  /** Release an object in the memo stack.
  @param object	object
  @param type	object type: MTR_MEMO_S_LOCK, ... */
  void memo_release(const void *object, ulint type);

  /** Release a page latch.
  @param[in]	ptr	pointer to within a page frame
  @param[in]	type	object type: MTR_MEMO_PAGE_X_FIX, ... */
  void release_page(const void *ptr, mtr_memo_type_t type);

  /** Note that the mini-transaction has modified data. */
  void set_modified() { m_impl.m_modifications = true; }

  /** Set the state to not-modified. This will not log the
  changes.  This is only used during redo log apply, to avoid
  logging the changes. */
  void discard_modifications() { m_impl.m_modifications = false; }

  /** Get the LSN of commit().
  @return the commit LSN
  @retval 0 if the transaction only modified temporary tablespaces */
  lsn_t commit_lsn() const MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(has_committed());
    ut_ad(m_impl.m_log_mode == MTR_LOG_ALL);
    return (m_commit_lsn);
  }

  /** Note that we are inside the change buffer code. */
  void enter_ibuf() { m_impl.m_inside_ibuf = true; }

  /** Note that we have exited from the change buffer code. */
  void exit_ibuf() { m_impl.m_inside_ibuf = false; }

  /** @return true if we are inside the change buffer code */
  bool is_inside_ibuf() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_impl.m_inside_ibuf);
  }

  /*
  @return true if the mini-transaction is active */
  bool is_active() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_impl.m_state == MTR_STATE_ACTIVE);
  }

  /** Get flush observer
  @return flush observer */
  FlushObserver *get_flush_observer() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_impl.m_flush_observer);
  }

  /** Set flush observer
  @param[in]	observer	flush observer */
  void set_flush_observer(FlushObserver *observer) {
    ut_ad(observer == NULL || m_impl.m_log_mode == MTR_LOG_NO_REDO);

    m_impl.m_flush_observer = observer;
  }

#ifdef UNIV_DEBUG
  /** Check if memo contains the given item.
  @param memo	memo stack
  @param object	object to search
  @param type	type of object
  @return	true if contains */
  static bool memo_contains(mtr_buf_t *memo, const void *object, ulint type)
      MY_ATTRIBUTE((warn_unused_result));

  /** Check if memo contains the given item.
  @param ptr		object to search
  @param flags		specify types of object (can be ORred) of
                          MTR_MEMO_PAGE_S_FIX ... values
  @return true if contains */
  bool memo_contains_flagged(const void *ptr, ulint flags) const
      MY_ATTRIBUTE((warn_unused_result));

  /** Check if memo contains the given page.
  @param[in]	ptr	pointer to within buffer frame
  @param[in]	flags	specify types of object with OR of
                          MTR_MEMO_PAGE_S_FIX... values
  @return	the block
  @retval	NULL	if not found */
  buf_block_t *memo_contains_page_flagged(const byte *ptr, ulint flags) const
      MY_ATTRIBUTE((warn_unused_result));

  /** Mark the given latched page as modified.
  @param[in]	ptr	pointer to within buffer frame */
  void memo_modify_page(const byte *ptr);

  /** Print info of an mtr handle. */
  void print() const;

  /** @return true if the mini-transaction has committed */
  bool has_committed() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_impl.m_state == MTR_STATE_COMMITTED);
  }

  /** @return true if the mini-transaction is committing */
  bool is_committing() const {
    return (m_impl.m_state == MTR_STATE_COMMITTING);
  }

  /** @return true if mini-transaction contains modifications. */
  bool has_modifications() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_impl.m_modifications);
  }

  /** @return the memo stack */
  const mtr_buf_t *get_memo() const MY_ATTRIBUTE((warn_unused_result)) {
    return (&m_impl.m_memo);
  }

  /** @return the memo stack */
  mtr_buf_t *get_memo() MY_ATTRIBUTE((warn_unused_result)) {
    return (&m_impl.m_memo);
  }
#endif /* UNIV_DEBUG */

  /** @return true if a record was added to the mini-transaction */
  bool is_dirty() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_impl.m_made_dirty);
  }

  /** Note that a record has been added to the log */
  void added_rec() { ++m_impl.m_n_log_recs; }

  /** Get the buffered redo log of this mini-transaction.
  @return	redo log */
  const mtr_buf_t *get_log() const MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);

    return (&m_impl.m_log);
  }

  /** Get the buffered redo log of this mini-transaction.
  @return	redo log */
  mtr_buf_t *get_log() MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);

    return (&m_impl.m_log);
  }

  /** Push an object to an mtr memo stack.
  @param object	object
  @param type	object type: MTR_MEMO_S_LOCK, ... */
  void memo_push(void *object, mtr_memo_type_t type);

  /** Check if this mini-transaction is dirtying a clean page.
  @param block	block being x-fixed
  @return true if the mtr is dirtying a clean page. */
  static bool is_block_dirtied(const buf_block_t *block)
      MY_ATTRIBUTE((warn_unused_result));

 private:
  Impl m_impl;

  /** LSN at commit time */
  lsn_t m_commit_lsn;

  /** true if it is synchronous mini-transaction */
  bool m_sync;

  class Command;

  friend class Command;
};
