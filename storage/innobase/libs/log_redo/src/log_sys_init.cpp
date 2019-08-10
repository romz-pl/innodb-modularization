#include <innodb/log_redo/log_sys_init.h>

#include <innodb/log_types/flags.h>
#include <innodb/log_buffer/log_allocate_buffer.h>
#include <innodb/log_chkp/log_allocate_checkpoint_buffer.h>
#include <innodb/log_files/log_allocate_file_header_buffers.h>
#include <innodb/log_flush/log_allocate_flush_events.h>
#include <innodb/log_redo/log_allocate_recent_closed.h>
#include <innodb/log_redo/log_allocate_recent_written.h>
#include <innodb/log_redo/log_allocate_write_ahead_buffer.h>
#include <innodb/log_redo/log_allocate_write_events.h>
#include <innodb/log_buffer/log_calc_buf_size.h>
#include <innodb/log_redo/log_calc_max_ages.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/log_redo/log_sys_object.h>
#include <innodb/log_types/log_header_format_t.h>
#include <innodb/log_files/log_files_update_offsets.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/log_redo/pfs.h>
#include <innodb/log_redo/srv_thread_concurrency.h>

extern const char *INNODB_PARAMETERS_MSG;

bool log_sys_init(uint32_t n_files, uint64_t file_size, space_id_t space_id) {
  ut_a(log_sys == nullptr);

  /* The log_sys_object is pointer to aligned_pointer. That's
  temporary solution until we refactor redo log more.

  That's required for now, because the aligned_pointer, has dtor
  which tries to free the memory and as long as this is global
  variable it will have the dtor called. However because we can
  exit without proper cleanup for redo log in some cases, we
  need to forbid dtor calls then. */

  log_sys_object = UT_NEW_NOKEY(aligned_pointer<log_t>{});

  log_sys_object->create();
  log_sys = *log_sys_object;

  log_t &log = *log_sys;

  /* Initialize simple value fields. */
  log.dict_persist_margin.store(0);
  log.periodical_checkpoints_enabled = false;
  log.format = LOG_HEADER_FORMAT_CURRENT;
  log.files_space_id = space_id;
  log.state = log_state_t::OK;
  log.n_log_ios_old = log.n_log_ios;
  log.last_printout_time = time(nullptr);

  ut_a(file_size <= std::numeric_limits<uint64_t>::max() / n_files);
  log.file_size = file_size;
  log.n_files = n_files;
  log.files_real_capacity = file_size * n_files;

  log.current_file_lsn = LOG_START_LSN;
  log.current_file_real_offset = LOG_FILE_HDR_SIZE;
  log_files_update_offsets(log, log.current_file_lsn);

  log.checkpointer_event = os_event_create("log_checkpointer_event");
  log.closer_event = os_event_create("log_closer_event");
  log.write_notifier_event = os_event_create("log_write_notifier_event");
  log.flush_notifier_event = os_event_create("log_flush_notifier_event");
  log.writer_event = os_event_create("log_writer_event");
  log.flusher_event = os_event_create("log_flusher_event");

  mutex_create(LATCH_ID_LOG_CHECKPOINTER, &log.checkpointer_mutex);
  mutex_create(LATCH_ID_LOG_CLOSER, &log.closer_mutex);
  mutex_create(LATCH_ID_LOG_WRITER, &log.writer_mutex);
  mutex_create(LATCH_ID_LOG_FLUSHER, &log.flusher_mutex);
  mutex_create(LATCH_ID_LOG_WRITE_NOTIFIER, &log.write_notifier_mutex);
  mutex_create(LATCH_ID_LOG_FLUSH_NOTIFIER, &log.flush_notifier_mutex);

  log.sn_lock.create(
#ifdef UNIV_PFS_RWLOCK
      log_sn_lock_key,
#else
      PSI_NOT_INSTRUMENTED,
#endif
      SYNC_LOG_SN, 64);

  /* Allocate buffers. */
  log_allocate_buffer(log);
  log_allocate_write_ahead_buffer(log);
  log_allocate_checkpoint_buffer(log);
  log_allocate_recent_written(log);
  log_allocate_recent_closed(log);
  log_allocate_flush_events(log);
  log_allocate_write_events(log);
  log_allocate_file_header_buffers(log);

  log_calc_buf_size(log);

  if (!log_calc_max_ages(log)) {
    ib::error(ER_IB_MSG_1267)
        << "Cannot continue operation. ib_logfiles are too"
        << " small for innodb_thread_concurrency " << srv_thread_concurrency
        << ". The combined size of"
        << " ib_logfiles should be bigger than"
        << " 200 kB * innodb_thread_concurrency. To get mysqld"
        << " to start up, set innodb_thread_concurrency in"
        << " my.cnf to a lower value, for example, to 8. After"
        << " an ERROR-FREE shutdown of mysqld you can adjust"
        << " the size of ib_logfiles. " << INNODB_PARAMETERS_MSG;

    return (false);
  }

  return (true);
}


