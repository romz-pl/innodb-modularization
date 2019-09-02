#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Gets the next explicit lock request on a record.
@param[in]	heap_no	heap number of the record
@param[in]	lock	lock
@return next lock, NULL if none exists or if heap_no == ULINT_UNDEFINED */
const lock_t *lock_rec_get_next_const(ulint heap_no, const lock_t *lock);
