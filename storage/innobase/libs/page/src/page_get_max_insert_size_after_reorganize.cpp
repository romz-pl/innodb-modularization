#include <innodb/page/page_get_max_insert_size_after_reorganize.h>

#include <innodb/page/page_get_data_size.h>
#include <innodb/page/page_dir_calc_reserved_space.h>
#include <innodb/page/page_get_free_space_of_empty.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_is_comp.h>

/** Returns the maximum combined size of records which can be inserted on top
 of the record heap if a page is first reorganized.
 @return maximum combined size for inserted records */
ulint page_get_max_insert_size_after_reorganize(
    const page_t *page, /*!< in: index page */
    ulint n_recs)       /*!< in: number of records */
{
  ulint occupied;
  ulint free_space;

  occupied = page_get_data_size(page) +
             page_dir_calc_reserved_space(n_recs + page_get_n_recs(page));

  free_space = page_get_free_space_of_empty(page_is_comp(page));

  if (occupied > free_space) {
    return (0);
  }

  return (free_space - occupied);
}
