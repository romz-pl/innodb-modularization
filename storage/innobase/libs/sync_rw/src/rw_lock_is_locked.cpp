#include <innodb/sync_rw/rw_lock_is_locked.h>

#ifdef UNIV_DEBUG

/** Checks if somebody has locked the rw-lock in the specified mode.
 @return true if locked */
bool rw_lock_is_locked(rw_lock_t *lock, /*!< in: rw-lock */
                       ulint lock_type) /*!< in: lock type: RW_LOCK_S,
                                        RW_LOCK_X or RW_LOCK_SX */
{
  ut_ad(rw_lock_validate(lock));

  switch (lock_type) {
    case RW_LOCK_S:
      return (rw_lock_get_reader_count(lock) > 0);

    case RW_LOCK_X:
      return (rw_lock_get_writer(lock) == RW_LOCK_X);

    case RW_LOCK_SX:
      return (rw_lock_get_sx_lock_count(lock) > 0);

    default:
      ut_error;
  }
}

#endif
