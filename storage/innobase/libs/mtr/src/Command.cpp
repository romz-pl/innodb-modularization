#include <innodb/mtr/Command.h>

#include <innodb/tablespace/recv_recovery_on.h>
#include <innodb/mtr/Release_all.h>
#include <innodb/mtr/Iterate.h>
#include <innodb/mtr/Add_dirty_blocks_to_flush_list.h>

extern bool recv_no_ibuf_operations;

void mlog_catenate_ulint(
    mtr_t *mtr,     /*!< in/out: mtr */
    ulint val,      /*!< in: value to write */
    mlog_id_t type);

/** Release the resources */
void mtr_t::Command::release_resources() {
  ut_ad(m_impl->m_magic_n == MTR_MAGIC_N);

  /* Currently only used in commit */
  ut_ad(m_impl->m_state == MTR_STATE_COMMITTING);

#ifdef UNIV_DEBUG
  Debug_check release;
  Iterate<Debug_check> iterator(release);

  m_impl->m_memo.for_each_block_in_reverse(iterator);
#endif /* UNIV_DEBUG */

  /* Reset the mtr buffers */
  m_impl->m_log.erase();

  m_impl->m_memo.erase();

  m_impl->m_state = MTR_STATE_COMMITTED;

  m_impl = 0;
}


