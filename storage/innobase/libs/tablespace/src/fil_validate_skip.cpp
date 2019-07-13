#include <innodb/tablespace/fil_validate_skip.h>

#ifdef UNIV_DEBUG

/** Try fil_validate() every this many times */
static const size_t FIL_VALIDATE_SKIP = 17;
/** Checks the consistency of the tablespace cache some of the time.
@return true if ok or the check was skipped */
bool fil_validate_skip() {
/** The fil_validate() call skip counter. Use a signed type
because of the race condition below. */
#ifdef UNIV_HOTBACKUP
  static meb::Mutex meb_mutex;

  meb_mutex.lock();
#endif /* UNIV_HOTBACKUP */
  static int fil_validate_count = FIL_VALIDATE_SKIP;

  /* There is a race condition below, but it does not matter,
  because this call is only for heuristic purposes. We want to
  reduce the call frequency of the costly fil_validate() check
  in debug builds. */
  --fil_validate_count;

  if (fil_validate_count > 0) {
#ifdef UNIV_HOTBACKUP
    meb_mutex.unlock();
#endif /* UNIV_HOTBACKUP */
    return (true);
  }

  fil_validate_count = FIL_VALIDATE_SKIP;
#ifdef UNIV_HOTBACKUP
  meb_mutex.unlock();
#endif /* UNIV_HOTBACKUP */

  return (fil_validate());
}

#endif
