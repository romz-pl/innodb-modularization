#include <innodb/log_write/log_refresh_stats.h>

#include <innodb/log_types/log_t.h>

void log_refresh_stats(log_t &log) {
  log.n_log_ios_old = log.n_log_ios;
  log.last_printout_time = time(nullptr);
}
