#include <innodb/buffer/LRUHp.h>

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/lst/lst.h>

/** Adjust the value of hp. This happens when some other thread working
on the same list attempts to remove the hp from the list.
@param bpage	buffer block to be compared */
void LRUHp::adjust(const buf_page_t *bpage) {
  ut_ad(bpage);

  /** We only support reverse traversal for now. */
  if (is_hp(bpage)) {
    m_hp = UT_LIST_GET_PREV(LRU, m_hp);
  }

  ut_ad(!m_hp || m_hp->in_LRU_list);
}
