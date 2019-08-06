#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

class mtr_t::Command {
 public:
  /** Constructor.
  Takes ownership of the mtr->m_impl, is responsible for deleting it.
  @param[in,out]	mtr	mini-transaction */
  explicit Command(mtr_t *mtr) : m_locks_released() { init(mtr); }

  void init(mtr_t *mtr) {
    m_impl = &mtr->m_impl;
    m_sync = mtr->m_sync;
  }

  /** Destructor */
  ~Command() { ut_ad(m_impl == 0); }

  /** Write the redo log record, add dirty pages to the flush list and
  release the resources. */
  void execute();

  /** Add blocks modified in this mini-transaction to the flush list. */
  void add_dirty_blocks_to_flush_list(lsn_t start_lsn, lsn_t end_lsn);

  /** Release both the latches and blocks used in the mini-transaction. */
  void release_all();

  /** Release the resources */
  void release_resources();

 private:
#ifndef UNIV_HOTBACKUP
  /** Prepare to write the mini-transaction log to the redo log buffer.
  @return number of bytes to write in finish_write() */
  ulint prepare_write();
#endif /* !UNIV_HOTBACKUP */

  /** true if it is a sync mini-transaction. */
  bool m_sync;

  /** The mini-transaction state. */
  mtr_t::Impl *m_impl;

  /** Set to 1 after the user thread releases the latches. The log
  writer thread must wait for this to be set to 1. */
  volatile ulint m_locks_released;
};
