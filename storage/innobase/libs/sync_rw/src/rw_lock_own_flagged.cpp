#include <innodb/sync_rw/rw_lock_own_flagged.h>

#ifdef UNIV_DEBUG

/** Checks if the thread has locked the rw-lock in the specified mode, with
the pass value == 0.
@param[in]	lock		rw-lock
@param[in]	flags		specify lock types with OR of the
                                rw_lock_flag_t values
@return true if locked */
bool rw_lock_own_flagged(const rw_lock_t *lock, rw_lock_flags_t flags) {
  Infos infos;

  rw_lock_get_debug_info(lock, &infos);

  Infos::const_iterator end = infos.end();

  for (Infos::const_iterator it = infos.begin(); it != end; ++it) {
    const rw_lock_debug_t *info = *it;

    ut_ad(os_thread_eq(info->thread_id, os_thread_get_curr_id()));

    if (info->pass != 0) {
      continue;
    }

    switch (info->lock_type) {
      case RW_LOCK_S:

        if (flags & RW_LOCK_FLAG_S) {
          return (true);
        }
        break;

      case RW_LOCK_X:

        if (flags & RW_LOCK_FLAG_X) {
          return (true);
        }
        break;

      case RW_LOCK_SX:

        if (flags & RW_LOCK_FLAG_SX) {
          return (true);
        }
    }
  }

  return (false);
}

#endif
