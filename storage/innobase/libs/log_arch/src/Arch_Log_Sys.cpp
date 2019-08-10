#include <innodb/log_arch/Arch_Log_Sys.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/disk/flags.h>
#include <innodb/ioasync/srv_shutdown_state.h>
#include <innodb/ioasync/srv_shutdown_t.h>
#include <innodb/log_arch/Arch_Group.h>
#include <innodb/log_arch/arch_log_sys.h>
#include <innodb/log_arch/flags.h>
#include <innodb/log_arch/flags.h>
#include <innodb/log_arch/log_archiver_thread_event.h>
#include <innodb/log_arch/log_archiver_thread_event.h>
#include <innodb/log_arch/srv_log_file_size.h>
#include <innodb/log_arch/srv_log_group_home_dir.h>
#include <innodb/log_arch/start_log_archiver_background.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_block/log_block_calc_checksum_crc32.h>
#include <innodb/log_block/log_block_get_checksum.h>
#include <innodb/log_block/log_block_set_checksum.h>
#include <innodb/log_types/log_checkpointer_mutex_enter.h>
#include <innodb/log_types/log_checkpointer_mutex_exit.h>
#include <innodb/log_files/log_files_header_fill.h>
#include <innodb/log_files/log_files_real_offset_for_lsn.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/machine/data.h>
// #include <innodb/log_buffer/log_buffer_get_last_block.h>

/** Chunk size for archiving redo log */
const uint ARCH_LOG_CHUNK_SIZE = 1024 * 1024;


void log_request_checkpoint(log_t &log, bool sync);
void log_buffer_get_last_block(log_t &log, lsn_t &last_lsn, byte *last_block,
                               uint32_t &block_len);

/** Update checkpoint LSN and related information in redo
log header block.
@param[in,out]	header		redo log header buffer
@param[in]	checkpoint_lsn	checkpoint LSN for recovery */
void Arch_Log_Sys::update_header(byte *header, lsn_t checkpoint_lsn) {
  lsn_t start_lsn;
  lsn_t lsn_offset;
  ib_uint64_t file_size;

  start_lsn = m_current_group->get_begin_lsn();
  file_size = m_current_group->get_file_size();

  start_lsn = ut_uint64_align_down(start_lsn, OS_FILE_LOG_BLOCK_SIZE);

  /* Copy Header information. */
  log_files_header_fill(header, start_lsn, LOG_HEADER_CREATOR_CLONE);

  ut_ad(checkpoint_lsn >= start_lsn);

  lsn_offset = checkpoint_lsn - start_lsn;
  file_size -= LOG_FILE_HDR_SIZE;

  lsn_offset = lsn_offset % file_size;
  lsn_offset += LOG_FILE_HDR_SIZE;

  /* Update checkpoint lsn and offset. */
  byte *src = header + LOG_CHECKPOINT_1;

  mach_write_to_8(src + LOG_CHECKPOINT_LSN, checkpoint_lsn);
  mach_write_to_8(src + LOG_CHECKPOINT_OFFSET, lsn_offset);
  log_block_set_checksum(src, log_block_calc_checksum_crc32(src));

  /* Copy to second checkpoint location */
  byte *dest = header + LOG_CHECKPOINT_2;

  memcpy(dest, src, OS_FILE_LOG_BLOCK_SIZE);
}

/** Start redo log archiving.
If archiving is already in progress, the client
is attached to current group.
@param[out]	group		log archive group
@param[out]	start_lsn	start lsn for client
@param[out]	header		redo log header
@param[in]	is_durable	if client needs durable archiving
@return error code */
int Arch_Log_Sys::start(Arch_Group *&group, lsn_t &start_lsn, byte *header,
                        bool is_durable) {
  bool create_new_group = false;

  memset(header, 0, LOG_FILE_HDR_SIZE);

  log_request_checkpoint(*log_sys, true);

  mutex_enter(&m_mutex);

  if (m_state == ARCH_STATE_ABORT) {
    mutex_exit(&m_mutex);
    my_error(ER_QUERY_INTERRUPTED, MYF(0));
    return (ER_QUERY_INTERRUPTED);
  }

  /* Start archiver task, if needed. */
  if (m_state == ARCH_STATE_INIT) {
    auto err = start_log_archiver_background();

    if (err != 0) {
      mutex_exit(&m_mutex);

      ib::error(ER_IB_MSG_17) << "Could not start Archiver"
                              << " background task";

      return (err);
    }
  }

  /* Start archiving from checkpoint LSN. */
  log_checkpointer_mutex_enter(*log_sys);
  log_writer_mutex_enter(*log_sys);

  start_lsn = log_sys->last_checkpoint_lsn;

  auto lsn_offset = log_files_real_offset_for_lsn(*log_sys, start_lsn);

  auto start_index = static_cast<uint>(lsn_offset / log_sys->file_size);
  uint64_t start_offset = lsn_offset % log_sys->file_size;

  /* Copy to first checkpoint location */
  auto src = static_cast<void *>(log_sys->checkpoint_buf);
  auto dest = static_cast<void *>(header + LOG_CHECKPOINT_1);

  memcpy(dest, src, OS_FILE_LOG_BLOCK_SIZE);

  /* Need to create a new group if archiving is not in progress. */
  if (m_state == ARCH_STATE_IDLE || m_state == ARCH_STATE_INIT) {
    m_archived_lsn.store(
        ut_uint64_align_down(start_lsn, OS_FILE_LOG_BLOCK_SIZE));
    create_new_group = true;
  }

  log_checkpointer_mutex_exit(*log_sys);

  /* Set archiver state to active. */
  if (m_state != ARCH_STATE_ACTIVE) {
    m_state = ARCH_STATE_ACTIVE;
    os_event_set(log_archiver_thread_event);
  }

  log_writer_mutex_exit(*log_sys);

  /* Create a new group. */
  if (create_new_group) {
    m_current_group = UT_NEW(Arch_Group(start_lsn, LOG_FILE_HDR_SIZE, &m_mutex),
                             mem_key_archive);

    if (m_current_group == nullptr) {
      mutex_exit(&m_mutex);

      my_error(ER_OUTOFMEMORY, MYF(0), sizeof(Arch_Group));
      return (ER_OUTOFMEMORY);
    }

    /* Currently use log file size for archived files. */
    auto db_err = m_current_group->init_file_ctx(
        ARCH_DIR, ARCH_LOG_DIR, ARCH_LOG_FILE, 0,
        static_cast<ib_uint64_t>(srv_log_file_size));

    if (db_err != DB_SUCCESS) {
      mutex_exit(&m_mutex);

      my_error(ER_OUTOFMEMORY, MYF(0), sizeof(Arch_File_Ctx));

      return (ER_OUTOFMEMORY);
    }

    start_offset = ut_uint64_align_down(start_offset, OS_FILE_LOG_BLOCK_SIZE);

    m_start_log_index = start_index;
    m_start_log_offset = start_offset;

    m_chunk_size = ARCH_LOG_CHUNK_SIZE;

    m_group_list.push_back(m_current_group);
  }

  /* Attach to the current group. */
  m_current_group->attach(is_durable);

  group = m_current_group;

  mutex_exit(&m_mutex);

  /* Update header with checkpoint LSN. */
  update_header(header, start_lsn);

  return (0);
}

/** Stop redo log archiving.
If other clients are there, the client is detached from
the current group.
@param[out]	group		log archive group
@param[out]	stop_lsn	stop lsn for client
@param[out]	log_blk		redo log trailer block
@param[in,out]	blk_len		length in bytes
@return error code */
int Arch_Log_Sys::stop(Arch_Group *group, lsn_t &stop_lsn, byte *log_blk,
                       uint32_t &blk_len) {
  int err = 0;

  if (log_blk == nullptr) {
    mutex_enter(&m_mutex);
    blk_len = 0;
    stop_lsn = m_archived_lsn.load();

  } else {
    /* Get the current LSN and trailer block. */
    log_buffer_get_last_block(*log_sys, stop_lsn, log_blk, blk_len);

    DBUG_EXECUTE_IF("clone_arch_log_stop_file_end",
                    group->adjust_end_lsn(stop_lsn, blk_len););

    /* Will throw error, if shutdown. We still continue
    with detach but return the error. */
    err = wait_archive_complete(stop_lsn);

    mutex_enter(&m_mutex);
  }

  auto count_active = group->detach(stop_lsn, nullptr);

  if (count_active == 0) {
    /* No other active client. Prepare to get idle. */
    log_writer_mutex_enter(*log_sys);

    if (m_state != ARCH_STATE_ABORT) {
      ut_ad(m_state == ARCH_STATE_ACTIVE);
      m_state = ARCH_STATE_PREPARE_IDLE;
      os_event_set(log_archiver_thread_event);
    }

    log_writer_mutex_exit(*log_sys);
  }

  mutex_exit(&m_mutex);

  return (err);
}

/** Force to abort the archiver (state becomes ARCH_STATE_ABORT). */
void Arch_Log_Sys::force_abort() {
  lsn_t lsn_max = LSN_MAX; /* unused */
  uint to_archive = 0;     /* unused */
  check_set_state(true, &lsn_max, &to_archive);
}

/** Release the current group from client.
@param[in]	group		group the client is attached to
@param[in]	is_durable	if client needs durable archiving */
void Arch_Log_Sys::release(Arch_Group *group, bool is_durable) {
  mutex_enter(&m_mutex);

  group->release(is_durable);

  /* Check if there are other references or archiving is still
  in progress. */
  if (group->is_referenced() || group->is_active()) {
    mutex_exit(&m_mutex);
    return;
  }

  /* Cleanup the group. */
  ut_ad(group != m_current_group);

  m_group_list.remove(group);

  UT_DELETE(group);

  mutex_exit(&m_mutex);
}

/** Check and set log archive system state and output the
amount of redo log available for archiving.
@param[in]	is_abort	need to abort
@param[in,out]	archived_lsn	LSN up to which redo log is archived
@param[out]	to_archive	amount of redo log to be archived */
Arch_State Arch_Log_Sys::check_set_state(bool is_abort, lsn_t *archived_lsn,
                                         uint *to_archive) {
  auto is_shutdown = (srv_shutdown_state == SRV_SHUTDOWN_LAST_PHASE ||
                      srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS);

  auto need_to_abort = (is_abort || is_shutdown);

  *to_archive = 0;

  mutex_enter(&m_mutex);

  switch (m_state) {
    case ARCH_STATE_ACTIVE:

      if (*archived_lsn != LSN_MAX) {
        /* Update system archived LSN from input */
        ut_ad(*archived_lsn >= m_archived_lsn.load());
        m_archived_lsn.store(*archived_lsn);
      } else {
        /* If input is not initialized,
        set from system archived LSN */
        *archived_lsn = m_archived_lsn.load();
      }

      lsn_t lsn_diff;

      /* Check redo log data ready to archive. */
      ut_ad(log_sys->write_lsn.load() >= m_archived_lsn.load());

      lsn_diff = log_sys->write_lsn.load() - m_archived_lsn.load();

      lsn_diff = ut_uint64_align_down(lsn_diff, OS_FILE_LOG_BLOCK_SIZE);

      /* Adjust archive data length if bigger than chunks size. */
      if (lsn_diff < m_chunk_size) {
        *to_archive = static_cast<uint>(lsn_diff);
      } else {
        *to_archive = m_chunk_size;
      }

      if (!need_to_abort) {
        break;
      }
      /* fall through */

    case ARCH_STATE_PREPARE_IDLE: {
      /* No active clients. Mark the group inactive and move
      to idle state. */
      m_current_group->disable(m_archived_lsn.load());

      /* If no client reference, free the group. */
      if (!m_current_group->is_referenced()) {
        m_group_list.remove(m_current_group);

        UT_DELETE(m_current_group);
      }

      m_current_group = nullptr;

      log_writer_mutex_enter(*log_sys);
      m_state = ARCH_STATE_IDLE;
      log_writer_mutex_exit(*log_sys);
    }
      /* fall through */

    case ARCH_STATE_IDLE:
    case ARCH_STATE_INIT:

      if (need_to_abort) {
        log_writer_mutex_enter(*log_sys);
        m_state = ARCH_STATE_ABORT;
        log_writer_mutex_exit(*log_sys);
      }
      break;

    case ARCH_STATE_ABORT:
      /* We could abort archiver from log_writer when
      it is already in the aborted state (shutdown). */
      break;

    default:
      ut_ad(false);
  }

  auto ret_state = m_state;
  mutex_exit(&m_mutex);

  return (ret_state);
}

/** Copy redo log from file context to archiver files.
@param[in]	file_ctx	file context for system redo logs
@param[in]	length		data to copy in bytes
@return error code */
dberr_t Arch_Log_Sys::copy_log(Arch_File_Ctx *file_ctx, uint length) {
  dberr_t err = DB_SUCCESS;

  if (file_ctx->is_closed()) {
    /* Open system redo log file context */
    err = file_ctx->open(true, LSN_MAX, m_start_log_index, m_start_log_offset);

    if (err != DB_SUCCESS) {
      return (err);
    }
  }

  Arch_Group *curr_group;
  uint write_size;

  curr_group = arch_log_sys->get_arch_group();

  /* Copy log data into one or more files in archiver group. */
  while (length > 0) {
    ib_uint64_t len_copy;
    ib_uint64_t len_left;

    len_copy = static_cast<ib_uint64_t>(length);

    len_left = file_ctx->bytes_left();

    /* Current file is over, switch to next file. */
    if (len_left == 0) {
      err = file_ctx->open_next(LSN_MAX, LOG_FILE_HDR_SIZE);
      if (err != DB_SUCCESS) {
        return (err);
      }

      len_left = file_ctx->bytes_left();
    }

    /* Write as much as possible from current file. */
    if (len_left < len_copy) {
      write_size = static_cast<uint>(len_left);
    } else {
      write_size = length;
    }

    err =
        curr_group->write_to_file(file_ctx, nullptr, write_size, false, false);

    if (err != DB_SUCCESS) {
      return (err);
    }

    ut_ad(length >= write_size);
    length -= write_size;
  }

  return (DB_SUCCESS);
}


/** Archive accumulated redo log in current group.
This interface is for archiver background task to archive redo log
data by calling it repeatedly over time.
@param[in, out]	init		true when called the first time; it will then
                                be set to false
@param[in]	curr_ctx	system redo logs to copy data from
@param[out]	arch_lsn	LSN up to which archiving is completed
@param[out]	wait		true, if no more redo to archive
@return true, if archiving is aborted */
bool Arch_Log_Sys::archive(bool init, Arch_File_Ctx *curr_ctx, lsn_t *arch_lsn,
                           bool *wait) {
  Arch_State curr_state;
  uint arch_len;

  dberr_t err = DB_SUCCESS;
  bool is_abort = false;

  /* Initialize system redo log file context first time. */
  if (init) {
    uint num_files;

    num_files = static_cast<uint>(log_sys->n_files);

    err = curr_ctx->init(srv_log_group_home_dir, nullptr, ib_logfile_basename,
                         num_files, log_sys->file_size);

    if (err != DB_SUCCESS) {
      is_abort = true;
    }
  }

  /* Find archive system state and amount of log data to archive. */
  curr_state = check_set_state(is_abort, arch_lsn, &arch_len);

  if (curr_state == ARCH_STATE_ABORT) {
    curr_ctx->close();
    return (true);

  } else if (curr_state == ARCH_STATE_IDLE || curr_state == ARCH_STATE_INIT) {
    curr_ctx->close();

    *arch_lsn = LSN_MAX;
    *wait = true;

    return (false);

  } else {
    ut_ad(curr_state == ARCH_STATE_ACTIVE);

    if (arch_len == 0) {
      /* Nothing to archive. Need to wait. */
      *wait = true;
      return (false);
    }

    /* Copy data from system redo log files to archiver files */
    err = copy_log(curr_ctx, arch_len);

    if (err == DB_SUCCESS) {
      *arch_lsn += arch_len;
      *wait = false;
      return (false);
    }

    /* Force abort in case of an error archiving data. */
    curr_state = check_set_state(true, arch_lsn, &arch_len);
    ut_ad(curr_state == ARCH_STATE_ABORT);

    return (true);
  }
}
