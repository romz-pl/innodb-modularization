#include <innodb/buffer/LRUItr.h>

#include <innodb/lst/lst.h>
#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buffer/buf_pool_t.h>

/** Selects from where to start a scan. If we have scanned too deep into
the LRU list it resets the value to the tail of the LRU list.
@return buf_page_t from where to start scan. */
buf_page_t *LRUItr::start() {
  ut_ad(mutex_own(m_mutex));

  if (!m_hp || m_hp->old) {
    m_hp = UT_LIST_GET_LAST(m_buf_pool->LRU);
  }

  return (m_hp);
}
