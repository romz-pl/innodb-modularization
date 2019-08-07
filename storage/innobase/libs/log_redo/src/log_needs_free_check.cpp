#include <innodb/log_redo/log_needs_free_check.h>

#include <innodb/log_redo/log_sys.h>

#ifndef UNIV_HOTBACKUP

bool log_needs_free_check() {
  const log_t &log = *log_sys;

  const sn_t sn = log.sn.load();

  return (sn > log.sn_limit_for_start.load());
}

#endif
