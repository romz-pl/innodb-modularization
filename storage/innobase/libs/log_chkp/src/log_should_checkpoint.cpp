#include <innodb/log_chkp/log_should_checkpoint.h>

#include <innodb/log_chkp/log_checkpoint_time_elapsed.h>
#include <innodb/log_chkp/srv_log_checkpoint_every.h>
#include <innodb/log_sn/log_translate_lsn_to_sn.h>
#include <innodb/log_sn/log_translate_sn_to_lsn.h>
#include <innodb/log_sn/lsn_t.h>
#include <innodb/log_types/log_free_check_margin.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/log_types/log_t.h>

bool log_should_checkpoint(log_t &log) {
  lsn_t last_checkpoint_lsn;
  lsn_t oldest_lsn;
  lsn_t current_lsn;
  lsn_t requested_checkpoint_lsn;
  uint64_t checkpoint_age;
  uint64_t checkpoint_time_elapsed;

  ut_ad(log_checkpointer_mutex_own(log));

#ifdef UNIV_DEBUG
  if (srv_checkpoint_disabled) {
    return (false);
  }
#endif /* UNIV_DEBUG */

  last_checkpoint_lsn = log.last_checkpoint_lsn.load();

  oldest_lsn = log.available_for_checkpoint_lsn;

  if (oldest_lsn <= last_checkpoint_lsn) {
    return (false);
  }

  requested_checkpoint_lsn = log.requested_checkpoint_lsn;

  current_lsn = log_get_lsn(log);

  ut_a(last_checkpoint_lsn <= oldest_lsn);
  ut_a(oldest_lsn <= current_lsn);

  const sn_t margin = log_free_check_margin(log);

  current_lsn =
      log_translate_sn_to_lsn(log_translate_lsn_to_sn(current_lsn) + margin);

  checkpoint_age = current_lsn - last_checkpoint_lsn;

  checkpoint_time_elapsed = log_checkpoint_time_elapsed(log);

  /* Update checkpoint_lsn stored in header of log files if:
          a) more than 1s elapsed since last checkpoint
          b) checkpoint age is greater than max_checkpoint_age_async
          c) it was requested to have greater checkpoint_lsn,
             and oldest_lsn allows to satisfy the request */

  bool periodical_checkpoint_disabled = false;

  DBUG_EXECUTE_IF("periodical_checkpoint_disabled",
                  periodical_checkpoint_disabled = true;);

  if ((log.periodical_checkpoints_enabled && !periodical_checkpoint_disabled &&
       checkpoint_time_elapsed >= srv_log_checkpoint_every * 1000ULL) ||
      checkpoint_age >= log.max_checkpoint_age_async ||
      (requested_checkpoint_lsn > last_checkpoint_lsn &&
       requested_checkpoint_lsn <= oldest_lsn)) {
    return (true);
  }

  return (false);
}
