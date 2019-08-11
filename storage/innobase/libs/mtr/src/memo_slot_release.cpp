#include <innodb/mtr/memo_slot_release.h>

#include <innodb/mtr/mtr_memo_slot_t.h>
#include <innodb/mtr/mtr_memo_type_t.h>
#include <innodb/buf_block/buf_block_unfix.h>
#include <innodb/buffer/buf_page_release_latch.h>
#include <innodb/sync_rw/rw_lock_s_unlock.h>
#include <innodb/sync_rw/rw_lock_sx_unlock.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>
#include <innodb/assert/assert.h>

/** Release latches and decrement the buffer fix count.
@param[in]	slot	memo slot */
void memo_slot_release(mtr_memo_slot_t *slot) {
  switch (slot->type) {
#ifndef UNIV_HOTBACKUP
    buf_block_t *block;
#endif /* !UNIV_HOTBACKUP */

    case MTR_MEMO_BUF_FIX:
    case MTR_MEMO_PAGE_S_FIX:
    case MTR_MEMO_PAGE_SX_FIX:
    case MTR_MEMO_PAGE_X_FIX:
#ifndef UNIV_HOTBACKUP
      block = reinterpret_cast<buf_block_t *>(slot->object);

      buf_block_unfix(block);
      buf_page_release_latch(block, slot->type);
#endif /* !UNIV_HOTBACKUP */
      break;

    case MTR_MEMO_S_LOCK:
      rw_lock_s_unlock(reinterpret_cast<rw_lock_t *>(slot->object));
      break;

    case MTR_MEMO_SX_LOCK:
      rw_lock_sx_unlock(reinterpret_cast<rw_lock_t *>(slot->object));
      break;

    case MTR_MEMO_X_LOCK:
      rw_lock_x_unlock(reinterpret_cast<rw_lock_t *>(slot->object));
      break;

#ifdef UNIV_DEBUG
    default:
      ut_ad(slot->type == MTR_MEMO_MODIFY);
#endif /* UNIV_DEBUG */
  }

  slot->object = NULL;
}
