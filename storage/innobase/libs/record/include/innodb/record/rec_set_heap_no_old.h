#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to set the heap number field in an old-style
record.
@param[in]	rec	physical record
@param[in]	heap_no	the heap number */
void rec_set_heap_no_old(rec_t *rec, ulint heap_no);
