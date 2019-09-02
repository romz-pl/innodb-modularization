#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** The back pointer to a waiting lock request in the transaction is set to NULL
 and the wait bit in lock type_mode is reset. */
UNIV_INLINE
void lock_reset_lock_and_trx_wait(lock_t *lock);
