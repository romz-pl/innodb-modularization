#include <innodb/clone/innodb_clone_end.h>

#include <innodb/clone/clone_sys.h>
#include <innodb/sync_mutex/IB_mutex_guard.h>

#include "mysql/plugin.h"  // thd_killed()

int innodb_clone_end(handlerton *hton, THD *thd, const byte *loc, uint loc_len,
                     uint task_id, int in_err) {
  /* Acquire clone system mutex which would automatically get released
  when we return from the function [RAII]. */
  IB_mutex_guard sys_mutex(clone_sys->get_mutex());

  /* Get clone handle by locator index. */
  auto clone_hdl = clone_sys->get_clone_by_index(loc, loc_len);

  /* Set error, if already not set */
  clone_hdl->save_error(in_err);

  /* Drop current task. */
  bool is_master = false;
  auto wait_reconnect = clone_hdl->drop_task(thd, task_id, in_err, is_master);
  auto is_copy = clone_hdl->is_copy_clone();
  auto is_init = clone_hdl->is_init();
  auto is_abort = clone_hdl->is_abort();

  if (!wait_reconnect || is_abort) {
    if (is_copy && is_master) {
      if (is_abort) {
        ib::info(ER_IB_MSG_151) << "Clone Master aborted by concurrent clone";

      } else if (in_err != 0) {
        /* Make sure re-start attempt fails immediately */
        clone_hdl->set_state(CLONE_STATE_ABORT);
      }
    }
    clone_sys->drop_clone(clone_hdl);

    ib::info(ER_IB_MSG_151)
        << "Clone"
        << (is_copy ? " End" : (is_init ? "Apply Version End" : " Apply End"))
        << (is_master ? " Master" : "") << " Task ID: " << task_id
        << (in_err != 0 ? " Failed, code: " : " Passed, code: ") << in_err;
    return (0);
  }

  ib::info(ER_IB_MSG_151) << "Clone Master wait for restart"
                          << " after n/w error code: " << in_err;

  ut_ad(clone_hdl->is_copy_clone());
  ut_ad(is_master);

  /* Set state to idle and wait for re-connect */
  clone_hdl->set_state(CLONE_STATE_IDLE);
  /* Sleep for 1 second */
  Clone_Msec sleep_time(Clone_Sec(1));
  /* Generate alert message every minute. */
  Clone_Sec alert_interval(Clone_Min(1));
  /* Wait for 5 minutes for client to reconnect back */
  Clone_Sec time_out(Clone_Min(5));

  bool is_timeout = false;
  auto err = Clone_Sys::wait(
      sleep_time, time_out, alert_interval,
      [&](bool alert, bool &result) {

        ut_ad(mutex_own(clone_sys->get_mutex()));
        result = !clone_hdl->is_active();

        if (thd_killed(thd) || clone_hdl->is_interrupted()) {
          ib::info(ER_IB_MSG_151)
              << "Clone End Master wait for Restart interrupted";
          my_error(ER_QUERY_INTERRUPTED, MYF(0));
          return (ER_QUERY_INTERRUPTED);

        } else if (Clone_Sys::s_clone_sys_state == CLONE_SYS_ABORT) {
          ib::info(ER_IB_MSG_151)
              << "Clone End Master wait for Restart aborted by DDL";
          my_error(ER_DDL_IN_PROGRESS, MYF(0));
          return (ER_DDL_IN_PROGRESS);

        } else if (clone_hdl->is_abort()) {
          result = false;
          ib::info(ER_IB_MSG_151) << "Clone End Master wait for Restart"
                                     " aborted by concurrent clone";
          return (0);
        }

        if (!result) {
          ib::info(ER_IB_MSG_151) << "Clone Master restarted successfully by "
                                     "other task after n/w failure";

        } else if (alert) {
          ib::info(ER_IB_MSG_151) << "Clone Master still waiting for restart";
        }
        return (0);
      },
      clone_sys->get_mutex(), is_timeout);

  if (err == 0 && is_timeout && clone_hdl->is_idle()) {
    ib::info(ER_IB_MSG_151) << "Clone End Master wait "
                               "for restart timed out after "
                               "5 Minutes. Dropping Snapshot";
  }
  /* Last task should drop the clone handle. */
  clone_sys->drop_clone(clone_hdl);
  return (0);
}

