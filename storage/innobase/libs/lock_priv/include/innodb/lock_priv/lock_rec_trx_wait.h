#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Reset the nth bit of a record lock.
@param[in,out]	lock record lock
@param[in] i	index of the bit that will be reset
@param[in] type	whether the lock is in wait mode  */
void lock_rec_trx_wait(lock_t *lock, ulint i, ulint type);
