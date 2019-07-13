#include <innodb/ioasync/os_aio_validate_skip.h>


#ifdef UNIV_DEBUG
#ifndef UNIV_HOTBACKUP

/** Validates the consistency the aio system some of the time.
@return true if ok or the check was skipped */
bool os_aio_validate_skip() {
/** Try os_aio_validate() every this many times */
#define OS_AIO_VALIDATE_SKIP 13

  /** The os_aio_validate() call skip counter.
  Use a signed type because of the race condition below. */
  static int os_aio_validate_count = OS_AIO_VALIDATE_SKIP;

  /* There is a race condition below, but it does not matter,
  because this call is only for heuristic purposes. We want to
  reduce the call frequency of the costly os_aio_validate()
  check in debug builds. */
  --os_aio_validate_count;

  if (os_aio_validate_count > 0) {
    return (true);
  }

  os_aio_validate_count = OS_AIO_VALIDATE_SKIP;
  return (os_aio_validate());
}

#endif /* !UNIV_HOTBACKUP */
#endif /* UNIV_DEBUG */
