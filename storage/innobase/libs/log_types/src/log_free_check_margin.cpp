#include <innodb/log_types/log_free_check_margin.h>

#include <innodb/log_types/log_t.h>

sn_t log_free_check_margin(const log_t &log) {
  sn_t margins = log.concurrency_margin.load();

  margins += log.dict_persist_margin.load();

  return (margins);
}
