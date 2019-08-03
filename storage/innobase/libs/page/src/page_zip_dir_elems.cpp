#include <innodb/page/page_zip_dir_elems.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/flag.h>

/** Gets the number of elements in the dense page directory,
 including deleted records (the free list).
 @return number of elements in the dense page directory */
ulint page_zip_dir_elems(
    const page_zip_des_t *page_zip) /*!< in: compressed page */
{
  /* Exclude the page infimum and supremum from the record count. */
  return (page_dir_get_n_heap(page_zip->data) - PAGE_HEAP_NO_USER_LOW);
}
