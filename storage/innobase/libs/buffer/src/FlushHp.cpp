#include <innodb/buffer/FlushHp.h>

#include <innodb/lst/lst.h>
#include <innodb/buf_page/buf_page_t.h>

/** Adjust the value of hp. This happens when some other thread working
on the same list attempts to remove the hp from the list.
@param bpage	buffer block to be compared */

void FlushHp::adjust(const buf_page_t *bpage) {
  ut_ad(bpage != NULL);

  /** We only support reverse traversal for now. */
  if (is_hp(bpage)) {
    m_hp = UT_LIST_GET_PREV(list, m_hp);
  }

  ut_ad(!m_hp || m_hp->in_flush_list);
}

