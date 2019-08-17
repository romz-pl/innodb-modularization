#include <innodb/mtr/mtr_t.h>

#include <innodb/mtr/Command.h>
#include <innodb/mtr/mtr_memo_slot_t.h>
#include <innodb/sync_rw/rw_lock_s_unlock.h>
#include <innodb/sync_rw/rw_lock_sx_lock.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>
#include <innodb/sync_rw/rw_lock_s_lock_inline.h>
#include <innodb/sync_rw/rw_lock_x_lock_inline.h>
#include <innodb/sync_rw/rw_lock_sx_lock_inline.h>
#include <innodb/buf_block/buf_block_unfix.h>
#include <innodb/buf_page/buf_page_release_latch.h>
#include <innodb/tablespace/mach_read_ulint.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/tablespace/fil_space_t.h>
#include <innodb/mtr/Find.h>
#include <innodb/mtr/Iterate.h>
#include <innodb/mtr/memo_slot_release.h>
#include <innodb/mtr/Find_page.h>



/**
Pushes an object to an mtr memo stack. */
void mtr_t::memo_push(void *object, mtr_memo_type_t type) {
  ut_ad(is_active());
  ut_ad(object != NULL);
  ut_ad(type >= MTR_MEMO_PAGE_S_FIX);
  ut_ad(type <= MTR_MEMO_SX_LOCK);
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);
  ut_ad(ut_is_2pow(type));

  /* If this mtr has x-fixed a clean page then we set
  the made_dirty flag. This tells us if we need
  insert the dirtied page to the flush list in mtr_commit. */

  if ((type == MTR_MEMO_PAGE_X_FIX || type == MTR_MEMO_PAGE_SX_FIX) &&
      !m_impl.m_made_dirty) {
    m_impl.m_made_dirty =
        is_block_dirtied(reinterpret_cast<const buf_block_t *>(object));
  } else if (type == MTR_MEMO_BUF_FIX && !m_impl.m_made_dirty) {
    if (reinterpret_cast<const buf_block_t *>(object)
            ->made_dirty_with_no_latch) {
      m_impl.m_made_dirty = true;
    }
  }

  mtr_memo_slot_t *slot;

  slot = m_impl.m_memo.push<mtr_memo_slot_t *>(sizeof(*slot));

  slot->type = type;
  slot->object = object;
}


/**
Releases the (index tree) s-latch stored in an mtr memo after a
savepoint. */
void mtr_t::release_s_latch_at_savepoint(ulint savepoint, rw_lock_t *lock) {
#ifndef UNIV_HOTBACKUP
  ut_ad(is_active());
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);

  ut_ad(m_impl.m_memo.size() > savepoint);

  mtr_memo_slot_t *slot = m_impl.m_memo.at<mtr_memo_slot_t *>(savepoint);

  ut_ad(slot->object == lock);
  ut_ad(slot->type == MTR_MEMO_S_LOCK);

  rw_lock_s_unlock(lock);

  slot->object = NULL;
#else  /* !UNIV_HOTBACKUP */
  UT_NOT_USED(savepoint);
  UT_NOT_USED(lock);
#endif /* !UNIV_HOTBACKUP */
}


#ifndef UNIV_HOTBACKUP
/**
SX-latches the not yet latched block after a savepoint. */

void mtr_t::sx_latch_at_savepoint(ulint savepoint, buf_block_t *block) {
  ut_ad(is_active());
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);
  ut_ad(m_impl.m_memo.size() > savepoint);

  ut_ad(!memo_contains_flagged(
      block, MTR_MEMO_PAGE_S_FIX | MTR_MEMO_PAGE_X_FIX | MTR_MEMO_PAGE_SX_FIX));

  mtr_memo_slot_t *slot;

  slot = m_impl.m_memo.at<mtr_memo_slot_t *>(savepoint);

  ut_ad(slot->object == block);

  /* == RW_NO_LATCH */
  ut_a(slot->type == MTR_MEMO_BUF_FIX);

  rw_lock_sx_lock(&block->lock);

  if (!m_impl.m_made_dirty) {
    m_impl.m_made_dirty = is_block_dirtied(block);
  }

  slot->type = MTR_MEMO_PAGE_SX_FIX;
}

/**
X-latches the not yet latched block after a savepoint. */

void mtr_t::x_latch_at_savepoint(ulint savepoint, buf_block_t *block) {
  ut_ad(is_active());
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);
  ut_ad(m_impl.m_memo.size() > savepoint);

  ut_ad(!memo_contains_flagged(
      block, MTR_MEMO_PAGE_S_FIX | MTR_MEMO_PAGE_X_FIX | MTR_MEMO_PAGE_SX_FIX));

  mtr_memo_slot_t *slot;

  slot = m_impl.m_memo.at<mtr_memo_slot_t *>(savepoint);

  ut_ad(slot->object == block);

  /* == RW_NO_LATCH */
  ut_a(slot->type == MTR_MEMO_BUF_FIX);

  rw_lock_x_lock(&block->lock);

  if (!m_impl.m_made_dirty) {
    m_impl.m_made_dirty = is_block_dirtied(block);
  }

  slot->type = MTR_MEMO_PAGE_X_FIX;
}

/**
Releases the block in an mtr memo after a savepoint. */

void mtr_t::release_block_at_savepoint(ulint savepoint, buf_block_t *block) {
  ut_ad(is_active());
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);

  mtr_memo_slot_t *slot;

  slot = m_impl.m_memo.at<mtr_memo_slot_t *>(savepoint);

  ut_a(slot->object == block);

  buf_block_unfix(reinterpret_cast<buf_block_t *>(block));

  buf_page_release_latch(block, slot->type);

  slot->object = NULL;
}
#endif /* !UNIV_HOTBACKUP */


/**
Gets the logging mode of a mini-transaction.
@return	logging mode: MTR_LOG_NONE, ... */

mtr_log_t mtr_t::get_log_mode() const {
  ut_ad(m_impl.m_log_mode >= MTR_LOG_ALL);
  ut_ad(m_impl.m_log_mode <= MTR_LOG_SHORT_INSERTS);

  return (m_impl.m_log_mode);
}

/**
Changes the logging mode of a mini-transaction.
@return	old mode */

mtr_log_t mtr_t::set_log_mode(mtr_log_t mode) {
  ut_ad(mode >= MTR_LOG_ALL);
  ut_ad(mode <= MTR_LOG_SHORT_INSERTS);

  const mtr_log_t old_mode = m_impl.m_log_mode;

  switch (old_mode) {
    case MTR_LOG_NO_REDO:
      /* Once this mode is set, it must not be changed. */
      ut_ad(mode == MTR_LOG_NO_REDO || mode == MTR_LOG_NONE);
      return (old_mode);
    case MTR_LOG_NONE:
      if (mode == old_mode || mode == MTR_LOG_SHORT_INSERTS) {
        /* Keep MTR_LOG_NONE. */
        return (old_mode);
      }
      /* fall through */
    case MTR_LOG_SHORT_INSERTS:
      ut_ad(mode == MTR_LOG_ALL);
      /* fall through */
    case MTR_LOG_ALL:
      /* MTR_LOG_NO_REDO can only be set before generating
      any redo log records. */
      ut_ad(mode != MTR_LOG_NO_REDO || m_impl.m_n_log_recs == 0);
      m_impl.m_log_mode = mode;
      return (old_mode);
  }

  ut_ad(0);
  return (old_mode);
}

#ifndef UNIV_HOTBACKUP
/**
Locks a lock in s-mode. */

void mtr_t::s_lock(rw_lock_t *lock, const char *file, ulint line) {
  rw_lock_s_lock_inline(lock, 0, file, line);

  memo_push(lock, MTR_MEMO_S_LOCK);
}

/**
Locks a lock in x-mode. */

void mtr_t::x_lock(rw_lock_t *lock, const char *file, ulint line) {
  rw_lock_x_lock_inline(lock, 0, file, line);

  memo_push(lock, MTR_MEMO_X_LOCK);
}

/**
Locks a lock in sx-mode. */

void mtr_t::sx_lock(rw_lock_t *lock, const char *file, ulint line) {
  rw_lock_sx_lock_inline(lock, 0, file, line);

  memo_push(lock, MTR_MEMO_SX_LOCK);
}
#endif /* !UNIV_HOTBACKUP */

/**
Reads 1 - 4 bytes from a file page buffered in the buffer pool.
@return	value read */

uint32_t mtr_t::read_ulint(const byte *ptr, mlog_id_t type) const {
  ut_ad(is_active());

  ut_ad(memo_contains_page_flagged(
      ptr, MTR_MEMO_PAGE_S_FIX | MTR_MEMO_PAGE_X_FIX | MTR_MEMO_PAGE_SX_FIX));

  return (mach_read_ulint(ptr, type));
}


/** Check if a mini-transaction is dirtying a clean page.
@return true if the mtr is dirtying a clean page. */
bool mtr_t::is_block_dirtied(const buf_block_t *block) {
  ut_ad(buf_block_get_state(block) == BUF_BLOCK_FILE_PAGE);
  ut_ad(block->page.buf_fix_count > 0);

  /* It is OK to read oldest_modification because no
  other thread can be performing a write of it and it
  is only during write that the value is reset to 0. */
  return (block->page.oldest_modification == 0);
}



/** Start a mini-transaction.
@param sync		true if it is a synchronous mini-transaction
@param read_only	true if read only mini-transaction */
void mtr_t::start(bool sync, bool read_only) {
  UNIV_MEM_INVALID(this, sizeof(*this));

  UNIV_MEM_INVALID(&m_impl, sizeof(m_impl));

  m_sync = sync;

  m_commit_lsn = 0;

  new (&m_impl.m_log) mtr_buf_t();
  new (&m_impl.m_memo) mtr_buf_t();

  m_impl.m_mtr = this;
  m_impl.m_log_mode = MTR_LOG_ALL;
  m_impl.m_inside_ibuf = false;
  m_impl.m_modifications = false;
  m_impl.m_made_dirty = false;
  m_impl.m_n_log_recs = 0;
  m_impl.m_state = MTR_STATE_ACTIVE;
  m_impl.m_flush_observer = NULL;

  ut_d(m_impl.m_magic_n = MTR_MAGIC_N);
}

/** Commit a mini-transaction. */
void mtr_t::commit() {
  ut_ad(is_active());
  ut_ad(!is_inside_ibuf());
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);
  m_impl.m_state = MTR_STATE_COMMITTING;

  Command cmd(this);

  if (m_impl.m_n_log_recs > 0 ||
      (m_impl.m_modifications && m_impl.m_log_mode == MTR_LOG_NO_REDO)) {
    ut_ad(!srv_read_only_mode || m_impl.m_log_mode == MTR_LOG_NO_REDO);

    cmd.execute();
  } else {
    cmd.release_all();
    cmd.release_resources();
  }
}

#ifndef UNIV_HOTBACKUP
/** Acquire a tablespace X-latch.
@param[in]	space		tablespace instance
@param[in]	file		file name from where called
@param[in]	line		line number in file */
void mtr_t::x_lock_space(fil_space_t *space, const char *file, ulint line) {
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);
  ut_ad(is_active());

  x_lock(&space->latch, file, line);
}

/** Release an object in the memo stack. */
void mtr_t::memo_release(const void *object, ulint type) {
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);
  ut_ad(is_active());

  /* We cannot release a page that has been written to in the
  middle of a mini-transaction. */
  ut_ad(!m_impl.m_modifications || type != MTR_MEMO_PAGE_X_FIX);

  Find find(object, type);
  Iterate<Find> iterator(find);

  if (!m_impl.m_memo.for_each_block_in_reverse(iterator)) {
    memo_slot_release(find.m_slot);
  }
}

/** Release a page latch.
@param[in]	ptr	pointer to within a page frame
@param[in]	type	object type: MTR_MEMO_PAGE_X_FIX, ... */
void mtr_t::release_page(const void *ptr, mtr_memo_type_t type) {
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);
  ut_ad(is_active());

  /* We cannot release a page that has been written to in the
  middle of a mini-transaction. */
  ut_ad(!m_impl.m_modifications || type != MTR_MEMO_PAGE_X_FIX);

  Find_page find(ptr, type);
  Iterate<Find_page> iterator(find);

  if (!m_impl.m_memo.for_each_block_in_reverse(iterator)) {
    memo_slot_release(find.get_slot());
    return;
  }

  /* The page was not found! */
  ut_ad(0);
}

#endif /* !UNIV_HOTBACKUP */


#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG
/** Check if memo contains the given item.
@return	true if contains */
bool mtr_t::memo_contains(mtr_buf_t *memo, const void *object, ulint type) {
  Find find(object, type);
  Iterate<Find> iterator(find);

  return (!memo->for_each_block_in_reverse(iterator));
}

/** Debug check for flags */
struct FlaggedCheck {
  FlaggedCheck(const void *ptr, ulint flags) : m_ptr(ptr), m_flags(flags) {
    // Do nothing
  }

  bool operator()(const mtr_memo_slot_t *slot) const {
    if (m_ptr == slot->object && (m_flags & slot->type)) {
      return (false);
    }

    return (true);
  }

  const void *m_ptr;
  ulint m_flags;
};

/** Check if memo contains the given item.
@param ptr		object to search
@param flags		specify types of object (can be ORred) of
                        MTR_MEMO_PAGE_S_FIX ... values
@return true if contains */
bool mtr_t::memo_contains_flagged(const void *ptr, ulint flags) const {
  ut_ad(m_impl.m_magic_n == MTR_MAGIC_N);
  ut_ad(is_committing() || is_active());

  FlaggedCheck check(ptr, flags);
  Iterate<FlaggedCheck> iterator(check);

  return (!m_impl.m_memo.for_each_block_in_reverse(iterator));
}

/** Check if memo contains the given page.
@param[in]	ptr	pointer to within buffer frame
@param[in]	flags	specify types of object with OR of
                        MTR_MEMO_PAGE_S_FIX... values
@return	the block
@retval	NULL	if not found */
buf_block_t *mtr_t::memo_contains_page_flagged(const byte *ptr,
                                               ulint flags) const {
  Find_page check(ptr, flags);
  Iterate<Find_page> iterator(check);

  return (m_impl.m_memo.for_each_block_in_reverse(iterator)
              ? NULL
              : check.get_block());
}

/** Mark the given latched page as modified.
@param[in]	ptr	pointer to within buffer frame */
void mtr_t::memo_modify_page(const byte *ptr) {
  buf_block_t *block = memo_contains_page_flagged(
      ptr, MTR_MEMO_PAGE_X_FIX | MTR_MEMO_PAGE_SX_FIX);
  ut_ad(block != NULL);

  if (!memo_contains(get_memo(), block, MTR_MEMO_MODIFY)) {
    memo_push(block, MTR_MEMO_MODIFY);
  }
}

/** Print info of an mtr handle. */
void mtr_t::print() const {
  ib::info(ER_IB_MSG_1275) << "Mini-transaction handle: memo size "
                           << m_impl.m_memo.size() << " bytes log size "
                           << get_log()->size() << " bytes";
}

#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */

