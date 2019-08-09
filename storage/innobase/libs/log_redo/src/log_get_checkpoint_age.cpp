#include <innodb/log_redo/log_get_checkpoint_age.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/log_get_lsn.h>

lsn_t log_get_checkpoint_age(const log_t &log) {
  const lsn_t last_checkpoint_lsn = log.last_checkpoint_lsn.load();

  const lsn_t current_lsn = log_get_lsn(log);

  if (current_lsn <= last_checkpoint_lsn) {
    /* Writes or reads have been somehow reordered.
    Note that this function does not provide any lock,
    and does not assume any lock existing. Therefore
    the calculated result is already outdated when the
    function is finished. Hence, we might assume that
    this time we calculated age = 0, because checkpoint
    lsn is close to current lsn if such race happened. */
    return (0);
  }

  return (current_lsn - last_checkpoint_lsn);
}

#endif
