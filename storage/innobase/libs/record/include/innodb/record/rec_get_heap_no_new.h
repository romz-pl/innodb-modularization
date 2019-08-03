#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to get the order number of a new-style
record in the heap of the index page.
@param[in]	rec	physical record
@return heap order number */
ulint rec_get_heap_no_new(const rec_t *rec) MY_ATTRIBUTE((warn_unused_result));
