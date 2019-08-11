#include <innodb/buffer/HazardPointer.h>

#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/buf_pool/buf_pool_from_bpage.h>
#include <innodb/buf_page/buf_page_in_file.h>
#include <innodb/buf_page/buf_page_get_state.h>


/** Set current value
@param bpage	buffer block to be set as hp */
void HazardPointer::set(buf_page_t *bpage) {
  ut_ad(mutex_own(m_mutex));
  ut_ad(!bpage || buf_pool_from_bpage(bpage) == m_buf_pool);
  ut_ad(!bpage || buf_page_in_file(bpage) ||
        buf_page_get_state(bpage) == BUF_BLOCK_REMOVE_HASH);

  m_hp = bpage;
}

/** Checks if a bpage is the hp
@param bpage    buffer block to be compared
@return true if it is hp */
bool HazardPointer::is_hp(const buf_page_t *bpage) {
  ut_ad(mutex_own(m_mutex));
  ut_ad(!m_hp || buf_pool_from_bpage(m_hp) == m_buf_pool);
  ut_ad(!bpage || buf_pool_from_bpage(bpage) == m_buf_pool);

  return (bpage == m_hp);
}

