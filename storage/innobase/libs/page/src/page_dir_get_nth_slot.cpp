#include <innodb/page/page_dir_get_nth_slot.h>

#ifdef UNIV_DEBUG
/** Gets pointer to nth directory slot.
 @return pointer to dir slot */
UNIV_INLINE
page_dir_slot_t *page_dir_get_nth_slot(
    const page_t *page, /*!< in: index page */
    ulint n)            /*!< in: position */
{
  ut_ad(page_dir_get_n_slots(page) > n);

  return ((page_dir_slot_t *)page + UNIV_PAGE_SIZE - PAGE_DIR -
          (n + 1) * PAGE_DIR_SLOT_SIZE);
}
#endif /* UNIV_DEBUG */
