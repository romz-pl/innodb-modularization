#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Allocates a block of memory from the heap of an index page.
 @return pointer to start of allocated buffer, or NULL if allocation fails */
byte *page_mem_alloc_heap(
    page_t *page,             /*!< in/out: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page with enough
                             space available for inserting the record,
                             or NULL */
    ulint need,               /*!< in: total number of bytes needed */
    ulint *heap_no);          /*!< out: this contains the heap number
                             of the allocated record
                             if allocation succeeds */
