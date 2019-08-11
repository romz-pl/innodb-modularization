#include <innodb/buffer/buf_page_is_old.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buf_page/buf_page_in_file.h>

/** Determine if a block has been flagged old.
@param[in]	bpage	control block
@return true if old */
ibool buf_page_is_old(const buf_page_t *bpage) {
#ifdef UNIV_DEBUG
  buf_pool_t *buf_pool = buf_pool_from_bpage(bpage);
  /* Buffer page mutex is not strictly required here for heuristic
  purposes even if LRU mutex is not being held.  Keep the assertion
  for now since all the callers hold it.  */
  ut_ad(mutex_own(buf_page_get_mutex(bpage)) ||
        mutex_own(&buf_pool->LRU_list_mutex));
#endif /* UNIV_DEBUG */
  ut_ad(buf_page_in_file(bpage));

  return (bpage->old);
}

#endif
