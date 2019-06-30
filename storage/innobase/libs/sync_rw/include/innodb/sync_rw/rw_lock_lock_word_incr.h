#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY

/** Increments lock_word the specified amount and returns new value.
@param[in,out]	lock		rw-lock
@param[in]	amount		amount to decrement
@return lock->lock_word after increment */
UNIV_INLINE
lint rw_lock_lock_word_incr(rw_lock_t *lock, ulint amount);

#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Increments lock_word the specified amount and returns new value.
 @return lock->lock_word after increment */
UNIV_INLINE
lint rw_lock_lock_word_incr(rw_lock_t *lock, /*!< in/out: rw-lock */
                            ulint amount)    /*!< in: amount of increment */
{
#ifdef INNODB_RW_LOCKS_USE_ATOMICS
  return (os_atomic_increment_lint(&lock->lock_word, amount));
#else  /* INNODB_RW_LOCKS_USE_ATOMICS */
  lint local_lock_word;

  mutex_enter(&(lock->mutex));

  lock->lock_word += amount;
  local_lock_word = lock->lock_word;

  mutex_exit(&(lock->mutex));

  return (local_lock_word);
#endif /* INNODB_RW_LOCKS_USE_ATOMICS */
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
