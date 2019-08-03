#include <innodb/page/page_get_free_space_of_empty.h>

#include <innodb/page/flag.h>
#include <innodb/page/header.h>

/** Calculates free space if a page is emptied.
 @return free space */
ulint page_get_free_space_of_empty(
    ulint comp) /*!< in: nonzero=compact page layout */
{
  if (comp) {
    return ((ulint)(UNIV_PAGE_SIZE - PAGE_NEW_SUPREMUM_END - PAGE_DIR -
                    2 * PAGE_DIR_SLOT_SIZE));
  }

  return ((ulint)(UNIV_PAGE_SIZE - PAGE_OLD_SUPREMUM_END - PAGE_DIR -
                  2 * PAGE_DIR_SLOT_SIZE));
}
