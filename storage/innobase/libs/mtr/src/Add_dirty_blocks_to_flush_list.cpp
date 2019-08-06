#include <innodb/mtr/Add_dirty_blocks_to_flush_list.h>


/** Constructor.
@param[in]	start_lsn	LSN of the first entry that was added
                                to REDO by the MTR
@param[in]	end_lsn		LSN after the last entry was added
                                to REDO by the MTR
@param[in,out]	observer	flush observer */
Add_dirty_blocks_to_flush_list::Add_dirty_blocks_to_flush_list(
    lsn_t start_lsn, lsn_t end_lsn, FlushObserver *observer)
    : m_end_lsn(end_lsn), m_start_lsn(start_lsn), m_flush_observer(observer) {
  /* Do nothing */
}
