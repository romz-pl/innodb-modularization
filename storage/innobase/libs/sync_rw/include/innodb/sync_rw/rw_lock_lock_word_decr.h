#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>


#ifndef UNIV_LIBRARY

/** Decrements lock_word the specified amount if it is greater than 0.
This is used by both s_lock and x_lock operations.
@param[in,out]	lock		rw-lock
@param[in]	amount		amount to decrement
@param[in]	threshold	threshold of judgement
@return true if decr occurs */
UNIV_INLINE
bool rw_lock_lock_word_decr(rw_lock_t *lock, ulint amount, lint threshold);

#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Two different implementations for decrementing the lock_word of a rw_lock:
 one for systems supporting atomic operations, one for others. This does
 does not support recusive x-locks: they should be handled by the caller and
 need not be atomic since they are performed by the current lock holder.
 Returns true if the decrement was made, false if not.
 @return true if decr occurs */
ALWAYS_INLINE
bool rw_lock_lock_word_decr(rw_lock_t *lock, /*!< in/out: rw-lock */
                            ulint amount,    /*!< in: amount to decrement */
                            lint threshold)  /*!< in: threshold of judgement */
{
#ifdef INNODB_RW_LOCKS_USE_ATOMICS
  lint local_lock_word;

  os_rmb;
  local_lock_word = lock->lock_word;
  while (local_lock_word > threshold) {
    if (os_compare_and_swap_lint(&lock->lock_word, local_lock_word,
                                 local_lock_word - amount)) {
      return (true);
    }
    local_lock_word = lock->lock_word;
  }
  return (false);
#else  /* INNODB_RW_LOCKS_USE_ATOMICS */
  bool success = false;
  mutex_enter(&(lock->mutex));
  if (lock->lock_word > threshold) {
    lock->lock_word -= amount;
    success = true;
  }
  mutex_exit(&(lock->mutex));
  return (success);
#endif /* INNODB_RW_LOCKS_USE_ATOMICS */
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
