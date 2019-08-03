#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/flag.h>

#ifdef UNIV_DEBUG
/** Gets pointer to nth directory slot.
@param[in]	page	index page
@param[in]	n	position
@return pointer to dir slot */
UNIV_INLINE
page_dir_slot_t *page_dir_get_nth_slot(const page_t *page, ulint n);
#else /* UNIV_DEBUG */
#define page_dir_get_nth_slot(page, n) \
  ((page) + (UNIV_PAGE_SIZE - PAGE_DIR - (n + 1) * PAGE_DIR_SLOT_SIZE))
#endif /* UNIV_DEBUG */
