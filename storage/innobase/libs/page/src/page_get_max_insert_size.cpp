#include <innodb/page/page_get_max_insert_size.h>

#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_header_get_field.h>
#include <innodb/page/page_dir_calc_reserved_space.h>
#include <innodb/page/header.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_get_free_space_of_empty.h>
#include <innodb/page/page_dir_get_n_heap.h>

/** Each user record on a page, and also the deleted user records in the heap
 takes its size plus the fraction of the dir cell size /
 PAGE_DIR_SLOT_MIN_N_OWNED bytes for it. If the sum of these exceeds the
 value of page_get_free_space_of_empty, the insert is impossible, otherwise
 it is allowed. This function returns the maximum combined size of records
 which can be inserted on top of the record heap.
 @return maximum combined size for inserted records */
ulint page_get_max_insert_size(const page_t *page, /*!< in: index page */
                               ulint n_recs)       /*!< in: number of records */
{
  ulint occupied;
  ulint free_space;

  if (page_is_comp(page)) {
    occupied =
        page_header_get_field(page, PAGE_HEAP_TOP) - PAGE_NEW_SUPREMUM_END +
        page_dir_calc_reserved_space(n_recs + page_dir_get_n_heap(page) - 2);

    free_space = page_get_free_space_of_empty(TRUE);
  } else {
    occupied =
        page_header_get_field(page, PAGE_HEAP_TOP) - PAGE_OLD_SUPREMUM_END +
        page_dir_calc_reserved_space(n_recs + page_dir_get_n_heap(page) - 2);

    free_space = page_get_free_space_of_empty(FALSE);
  }

  /* Above the 'n_recs +' part reserves directory space for the new
  inserted records; the '- 2' excludes page infimum and supremum
  records */

  if (occupied > free_space) {
    return (0);
  }

  return (free_space - occupied);
}
