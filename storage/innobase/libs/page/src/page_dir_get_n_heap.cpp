#include <innodb/page/page_dir_get_n_heap.h>

#include <innodb/page/header.h>
#include <innodb/page/page_header_get_field.h>

/** Gets the number of records in the heap.
 @return number of user records */
ulint page_dir_get_n_heap(const page_t *page) /*!< in: index page */
{
  return (page_header_get_field(page, PAGE_N_HEAP) & 0x7fff);
}
