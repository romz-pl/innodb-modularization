#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** This function is a wrapper around several functions which need to be called
in particular order to wake up a transaction waiting for a lock.
You should not call lock_wait_release_thread_if_suspended(thr) directly,
but rather use this wrapper, as this makes it much easier to reason about all
possible states in which lock, trx, and thr can be.
It makes sure that trx is woken up exactly once, and only if it already went to
sleep.
@param[in, out]   lock    The lock for which lock->trx is waiting */
void lock_reset_wait_and_release_thread_if_suspended(lock_t *lock);
