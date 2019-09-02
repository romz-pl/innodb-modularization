#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;

/** Gets the heap_no of the smallest user record on a page.
 @return heap_no of smallest user record, or PAGE_HEAP_NO_SUPREMUM */
UNIV_INLINE
ulint lock_get_min_heap_no(const buf_block_t *block); /*!< in: buffer block */
