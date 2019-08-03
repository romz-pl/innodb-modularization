#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Returns the heap number of a record.
 @return heap number */
ulint page_rec_get_heap_no(const rec_t *rec); /*!< in: the physical record */
