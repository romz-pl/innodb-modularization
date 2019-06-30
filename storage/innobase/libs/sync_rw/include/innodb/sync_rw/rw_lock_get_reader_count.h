#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/X_LOCK_DECR.h>
#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>

#ifndef UNIV_LIBRARY

/** Returns the number of readers (s-locks).
 @return number of readers */
UNIV_INLINE
ulint rw_lock_get_reader_count(const rw_lock_t *lock); /*!< in: rw-lock */

#endif /* UNIV_LIBRARY */



#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Returns the number of readers (s-locks).
 @return number of readers */
UNIV_INLINE
ulint rw_lock_get_reader_count(const rw_lock_t *lock) /*!< in: rw-lock */
{
  lint lock_word = lock->lock_word;
  ut_ad(lock_word <= X_LOCK_DECR);

  if (lock_word > X_LOCK_HALF_DECR) {
    /* s-locked, no x-waiter */
    return (X_LOCK_DECR - lock_word);
  } else if (lock_word > 0) {
    /* s-locked, with sx-locks only */
    return (X_LOCK_HALF_DECR - lock_word);
  } else if (lock_word == 0) {
    /* x-locked */
    return (0);
  } else if (lock_word > -X_LOCK_HALF_DECR) {
    /* s-locked, with x-waiter */
    return ((ulint)(-lock_word));
  } else if (lock_word == -X_LOCK_HALF_DECR) {
    /* x-locked with sx-locks */
    return (0);
  } else if (lock_word > -X_LOCK_DECR) {
    /* s-locked, with x-waiter and sx-lock */
    return ((ulint)(-(lock_word + X_LOCK_HALF_DECR)));
  }
  /* no s-locks */
  return (0);
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
