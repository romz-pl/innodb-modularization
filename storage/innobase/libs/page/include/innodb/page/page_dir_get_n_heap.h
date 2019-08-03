#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Gets the number of records in the heap.
 @return number of user records */
ulint page_dir_get_n_heap(const page_t *page); /*!< in: index page */
