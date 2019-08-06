#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_memo_slot_t.h>
#include <innodb/buffer/buf_block_t.h>
#include <innodb/mtr/mtr_memo_type_t.h>

static_assert(static_cast<int>(MTR_MEMO_PAGE_S_FIX) ==
                  static_cast<int>(RW_S_LATCH),
              "");

static_assert(static_cast<int>(MTR_MEMO_PAGE_X_FIX) ==
                  static_cast<int>(RW_X_LATCH),
              "");

static_assert(static_cast<int>(MTR_MEMO_PAGE_SX_FIX) ==
                  static_cast<int>(RW_SX_LATCH),
              "");


/** Find a page frame */
struct Find_page {
  /** Constructor
  @param[in]	ptr	pointer to within a page frame
  @param[in]	flags	MTR_MEMO flags to look for */
  Find_page(const void *ptr, ulint flags)
      : m_ptr(ptr), m_flags(flags), m_slot(NULL) {
    /* We can only look for page-related flags. */
    ut_ad(!(flags &
            ~(MTR_MEMO_PAGE_S_FIX | MTR_MEMO_PAGE_X_FIX | MTR_MEMO_PAGE_SX_FIX |
              MTR_MEMO_BUF_FIX | MTR_MEMO_MODIFY)));
  }

  /** Visit a memo entry.
  @param[in]	slot	memo entry to visit
  @retval	false	if a page was found
  @retval	true	if the iteration should continue */
  bool operator()(mtr_memo_slot_t *slot) {
    ut_ad(m_slot == NULL);

    if (!(m_flags & slot->type) || slot->object == NULL) {
      return (true);
    }

    buf_block_t *block = reinterpret_cast<buf_block_t *>(slot->object);

    if (m_ptr < block->frame ||
        m_ptr >= block->frame + block->page.size.logical()) {
      return (true);
    }

    m_slot = slot;
    return (false);
  }

  /** @return the slot that was found */
  mtr_memo_slot_t *get_slot() const {
    ut_ad(m_slot != NULL);
    return (m_slot);
  }
  /** @return the block that was found */
  buf_block_t *get_block() const {
    return (reinterpret_cast<buf_block_t *>(get_slot()->object));
  }

 private:
  /** Pointer inside a page frame to look for */
  const void *const m_ptr;
  /** MTR_MEMO flags to look for */
  const ulint m_flags;
  /** The slot corresponding to m_ptr */
  mtr_memo_slot_t *m_slot;
};
