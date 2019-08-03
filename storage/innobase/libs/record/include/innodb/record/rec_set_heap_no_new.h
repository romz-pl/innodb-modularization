#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to set the heap number field in a new-style
record.
@param[in,out]	rec	physical record
@param[in]	heap_no	the heap number */
void rec_set_heap_no_new(rec_t *rec, ulint heap_no);
