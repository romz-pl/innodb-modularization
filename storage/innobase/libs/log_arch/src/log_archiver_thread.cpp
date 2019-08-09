#include <innodb/log_arch/log_archiver_thread.h>

#include <innodb/log_arch/Arch_File_Ctx.h>
#include <innodb/log_arch/arch_log_sys.h>
#include <innodb/sync_event/os_event_wait.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/log_arch/log_archiver_is_active.h>
#include <innodb/logger/info.h>
#include <innodb/log_arch/log_archiver_thread_event.h>

/** Archiver background thread */
void log_archiver_thread() {
  Arch_File_Ctx log_file_ctx;
  lsn_t log_arch_lsn = LSN_MAX;

  bool log_abort = false;
  bool log_wait = false;
  bool log_init = true;

  while (true) {
    /* Archive available redo log data. */
    log_abort = arch_log_sys->archive(log_init, &log_file_ctx, &log_arch_lsn,
                                      &log_wait);

    if (log_abort) {
      ib::info(ER_IB_MSG_13) << "Exiting Log Archiver";
      break;
    }

    log_init = false;

    if (log_wait) {
      /* Nothing to archive. Wait until next trigger. */
      os_event_wait(log_archiver_thread_event);
      os_event_reset(log_archiver_thread_event);
    }
  }

  log_archiver_is_active = false;
}
