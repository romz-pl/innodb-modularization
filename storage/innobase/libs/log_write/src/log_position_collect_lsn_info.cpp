#include <innodb/log_write/log_position_collect_lsn_info.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_buffer/log_buffer_x_lock_own.h>
#include <innodb/log_chkp/log_checkpointer_mutex_own.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/log_types/flags.h>

void log_position_collect_lsn_info(const log_t &log, lsn_t *current_lsn,
                                   lsn_t *checkpoint_lsn) {
  ut_ad(log_buffer_x_lock_own(log));
  ut_ad(log_checkpointer_mutex_own(log));

  *checkpoint_lsn = log.last_checkpoint_lsn.load();

  *current_lsn = log_get_lsn(log);

  /* Ensure we have redo log started. */
  ut_a(*current_lsn >= LOG_START_LSN);
  ut_a(*checkpoint_lsn >= LOG_START_LSN);

  /* Obviously current lsn cannot point to before checkpoint. */
  ut_a(*current_lsn >= *checkpoint_lsn);
}
