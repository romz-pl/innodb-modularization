#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/buf_block_t.h>
#include <innodb/mtr/mtr_memo_slot_t.h>
#include <innodb/mtr/mtr_memo_type_t.h>
#include <innodb/tablespace/lsn_t.h>

class FlushObserver;

void buf_flush_note_modification(buf_block_t *block, lsn_t start_lsn,
                                 lsn_t end_lsn, FlushObserver *observer);

/** Add blocks modified by the mini-transaction to the flush list. */
struct Add_dirty_blocks_to_flush_list {
  /** Constructor.
  @param[in]	start_lsn	LSN of the first entry that was
                                  added to REDO by the MTR
  @param[in]	end_lsn		LSN after the last entry was
                                  added to REDO by the MTR
  @param[in,out]	observer	flush observer */
  Add_dirty_blocks_to_flush_list(lsn_t start_lsn, lsn_t end_lsn,
                                 FlushObserver *observer);

  /** Add the modified page to the buffer flush list. */
  void add_dirty_page_to_flush_list(mtr_memo_slot_t *slot) const {
    ut_ad(m_end_lsn > m_start_lsn || (m_end_lsn == 0 && m_start_lsn == 0));

#ifndef UNIV_HOTBACKUP
    buf_block_t *block;

    block = reinterpret_cast<buf_block_t *>(slot->object);

    buf_flush_note_modification(block, m_start_lsn, m_end_lsn,
                                m_flush_observer);
#endif /* !UNIV_HOTBACKUP */
  }

  /** @return true always. */
  bool operator()(mtr_memo_slot_t *slot) const {
    if (slot->object != NULL) {
      if (slot->type == MTR_MEMO_PAGE_X_FIX ||
          slot->type == MTR_MEMO_PAGE_SX_FIX) {
        add_dirty_page_to_flush_list(slot);

      } else if (slot->type == MTR_MEMO_BUF_FIX) {
        buf_block_t *block;
        block = reinterpret_cast<buf_block_t *>(slot->object);
        if (block->made_dirty_with_no_latch) {
          add_dirty_page_to_flush_list(slot);
          block->made_dirty_with_no_latch = false;
        }
      }
    }

    return (true);
  }

  /** Mini-transaction REDO end LSN */
  const lsn_t m_end_lsn;

  /** Mini-transaction REDO start LSN */
  const lsn_t m_start_lsn;

  /** Flush observer */
  FlushObserver *const m_flush_observer;
};
