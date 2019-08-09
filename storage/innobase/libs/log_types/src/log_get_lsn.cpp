#include <innodb/log_types/log_get_lsn.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/log_translate_sn_to_lsn.h>

lsn_t log_get_lsn(const log_t &log) {
  return (log_translate_sn_to_lsn(log.sn.load()));
}

#endif
