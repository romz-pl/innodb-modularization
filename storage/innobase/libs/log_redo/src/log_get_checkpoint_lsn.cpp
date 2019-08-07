#include <innodb/log_redo/log_get_checkpoint_lsn.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/log_t.h>

lsn_t log_get_checkpoint_lsn(const log_t &log) {
  return (log.last_checkpoint_lsn.load());
}

#endif
