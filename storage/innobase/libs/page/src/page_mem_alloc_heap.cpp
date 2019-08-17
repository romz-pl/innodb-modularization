#include <innodb/page/page_mem_alloc_heap.h>

#include <innodb/page/header.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_dir_set_n_heap.h>
#include <innodb/page/page_get_max_insert_size.h>
#include <innodb/page/page_header_get_ptr.h>
#include <innodb/page/page_header_set_ptr.h>
#include <innodb/page/page_zip_des_t.h>

/** Allocates a block of memory from the heap of an index page.
 @return pointer to start of allocated buffer, or NULL if allocation fails */
byte *page_mem_alloc_heap(
    page_t *page,             /*!< in/out: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page with enough
                             space available for inserting the record,
                             or NULL */
    ulint need,               /*!< in: total number of bytes needed */
    ulint *heap_no)           /*!< out: this contains the heap number
                              of the allocated record
                              if allocation succeeds */
{
  byte *block;
  ulint avl_space;

  ut_ad(page && heap_no);

  avl_space = page_get_max_insert_size(page, 1);

  if (avl_space >= need) {
    block = page_header_get_ptr(page, PAGE_HEAP_TOP);

    page_header_set_ptr(page, page_zip, PAGE_HEAP_TOP, block + need);
    *heap_no = page_dir_get_n_heap(page);

    page_dir_set_n_heap(page, page_zip, 1 + *heap_no);

    return (block);
  }

  return (NULL);
}

