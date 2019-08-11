#include <innodb/clone/Clone_Handle.h>

#include <innodb/buf_page/flags.h>
#include <innodb/clone/Clone_Desc_Data.h>
#include <innodb/clone/Clone_Desc_File_MetaData.h>
#include <innodb/clone/Clone_Desc_Task_Meta.h>
#include <innodb/clone/clone_sys.h>
#include <innodb/io/Fil_path.h>
#include <innodb/io/create_purpose.h>
#include <innodb/io/create_type.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_create.h>
#include <innodb/io/os_file_create_directory.h>
#include <innodb/io/os_file_create_subdirs_if_needed.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/os_file_delete.h>
#include <innodb/io/os_file_flush.h>
#include <innodb/io/os_file_get_size.h>
#include <innodb/io/os_file_seek.h>
#include <innodb/io/os_file_status.h>
#include <innodb/io/os_file_type_t.h>
#include <innodb/io/pfs.h>
#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/io/srv_is_direct_io.h>
#include <innodb/ioasync/os_file_set_size.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fsp_is_ibd_tablespace.h>
#include <innodb/time/ut_time_ms.h>

#include "mysql/plugin.h"  // thd_killed()

Clone_Handle::Clone_Handle(Clone_Handle_Type handle_type, uint clone_version,
                           uint clone_index)
    : m_clone_handle_type(handle_type),
      m_clone_handle_state(CLONE_STATE_INIT),
      m_clone_locator(),
      m_locator_length(),
      m_restart_loc(),
      m_restart_loc_len(),
      m_clone_desc_version(clone_version),
      m_clone_arr_index(clone_index),
      m_clone_id(),
      m_ref_count(),
      m_allow_restart(false),
      m_clone_dir(),
      m_clone_task_manager() {
  mutex_create(LATCH_ID_CLONE_TASK, m_clone_task_manager.get_mutex());

  Clone_Desc_Locator loc_desc;
  loc_desc.init(0, 0, CLONE_SNAPSHOT_NONE, clone_version, clone_index);

  auto loc = &m_version_locator[0];
  uint len = CLONE_DESC_MAX_BASE_LEN;

  memset(loc, 0, CLONE_DESC_MAX_BASE_LEN);

  loc_desc.serialize(loc, len, nullptr, nullptr);

  ut_ad(len <= CLONE_DESC_MAX_BASE_LEN);
}

Clone_Handle::~Clone_Handle() {
  mutex_free(m_clone_task_manager.get_mutex());

  if (!is_init()) {
    clone_sys->detach_snapshot(m_clone_task_manager.get_snapshot(),
                               m_clone_handle_type);
  }
  ut_ad(m_ref_count == 0);
}

void Clone_Handle::delete_clone_file() {
  ut_ad(!is_copy_clone());
  /* Delete clone in progress file on successful completion */
  char file_name[FN_REFLEN + CLONE_FILE_LEN + 1];

  snprintf(file_name, FN_REFLEN + CLONE_FILE_LEN + 1, "%s%c%s", m_clone_dir,
           OS_PATH_SEPARATOR, CLONE_IN_PROGRESS_FILE);

  os_file_delete(innodb_clone_file_key, file_name);
}

int Clone_Handle::create_clone_directory() {
  ut_ad(!is_copy_clone());

  char errbuf[MYSYS_STRERROR_SIZE];
  char file_name[FN_REFLEN + CLONE_FILE_LEN + 1];

  /* Create data directory for clone. */
  auto db_err = os_file_create_subdirs_if_needed(m_clone_dir);

  if (db_err == DB_SUCCESS) {
    auto status = os_file_create_directory(m_clone_dir, false);

    if (status) {
      /* Mark clone in progress */
      snprintf(file_name, FN_REFLEN + CLONE_FILE_LEN + 1, "%s%c%s", m_clone_dir,
               OS_PATH_SEPARATOR, CLONE_IN_PROGRESS_FILE);

      auto file =
          os_file_create(innodb_clone_file_key, file_name,
                         OS_FILE_CREATE | OS_FILE_ON_ERROR_NO_EXIT,
                         OS_FILE_NORMAL, OS_CLONE_LOG_FILE, false, &status);
      if (status) {
        os_file_close(file);
      }
    }

    /* Create mysql schema directory. */
    if (status) {
      snprintf(file_name, FN_REFLEN + CLONE_FILE_LEN + 1, "%s%cmysql",
               m_clone_dir, OS_PATH_SEPARATOR);

      status = os_file_create_directory(file_name, true);
    }

    if (!status) {
      db_err = DB_ERROR;
    }
  }

  if (db_err != DB_SUCCESS) {
    my_error(ER_CANT_CREATE_DB, MYF(0), m_clone_dir, errno,
             my_strerror(errbuf, sizeof(errbuf), errno));

    return (ER_CANT_CREATE_DB);
  }
  return (0);
}

int Clone_Handle::init(const byte *ref_loc, uint ref_len, Ha_clone_type type,
                       const char *data_dir) {
  ib_uint64_t snapshot_id;
  Clone_Snapshot *snapshot;

  m_clone_dir = data_dir;

  bool enable_monitor = true;

  /* Generate unique clone identifiers for copy clone handle. */
  if (is_copy_clone()) {
    m_clone_id = clone_sys->get_next_id();
    snapshot_id = clone_sys->get_next_id();

    /* For local clone, monitor while applying data. */
    if (ref_loc == nullptr) {
      enable_monitor = false;
    }

  } else {
    /* Return keeping the clone in INIT state. The locator
    would only have the version information. */
    if (ref_loc == nullptr) {
      return (0);
    }

    auto err = create_clone_directory();
    if (err != 0) {
      return (err);
    }

    /* Set clone identifiers from reference locator for apply clone
    handle. The reference locator is from copy clone handle. */
    Clone_Desc_Locator loc_desc;

    loc_desc.deserialize(ref_loc, ref_len, nullptr);

    m_clone_id = loc_desc.m_clone_id;
    snapshot_id = loc_desc.m_snapshot_id;

    ut_ad(m_clone_id != CLONE_LOC_INVALID_ID);
    ut_ad(snapshot_id != CLONE_LOC_INVALID_ID);
  }

  /* Create and attach to snapshot. */
  auto err = clone_sys->attach_snapshot(m_clone_handle_type, type, snapshot_id,
                                        enable_monitor, snapshot);

  if (err != 0) {
    return (err);
  }

  /* Initialize clone task manager. */
  m_clone_task_manager.init(snapshot);

  m_clone_handle_state = CLONE_STATE_ACTIVE;

  return (0);
}

byte *Clone_Handle::get_locator(uint &loc_len) {
  Clone_Desc_Locator loc_desc;

  /* Return version locator during initialization. */
  if (is_init()) {
    loc_len = CLONE_DESC_MAX_BASE_LEN;
    return (&m_version_locator[0]);
  }

  auto snapshot = m_clone_task_manager.get_snapshot();

  auto heap = snapshot->lock_heap();

  build_descriptor(&loc_desc);

  loc_desc.serialize(m_clone_locator, m_locator_length, nullptr, heap);

  loc_len = m_locator_length;

  snapshot->release_heap(heap);

  return (m_clone_locator);
}

void Clone_Handle::build_descriptor(Clone_Desc_Locator *loc_desc) {
  Clone_Snapshot *snapshot;
  ib_uint64_t snapshot_id = CLONE_LOC_INVALID_ID;
  Snapshot_State state = CLONE_SNAPSHOT_NONE;

  snapshot = m_clone_task_manager.get_snapshot();

  if (snapshot) {
    state = snapshot->get_state();
    snapshot_id = snapshot->get_id();
  }

  loc_desc->init(m_clone_id, snapshot_id, state, m_clone_desc_version,
                 m_clone_arr_index);
}

bool Clone_Handle::drop_task(THD *thd, uint task_id, int in_err,
                             bool &is_master) {
  /* No task is added in INIT state. The drop task is still called and
  should be ignored. */
  if (is_init()) {
    /* Only relevant for apply clone master */
    ut_ad(!is_copy_clone());
    ut_ad(task_id == 0);
    is_master = true;
    return (false);
  }
  /* Cannot be in IDLE state as master waits for tasks to drop before idling */
  ut_ad(!is_idle());

  /* Close and reset file related information */
  auto task = m_clone_task_manager.get_task_by_index(task_id);

  close_file(task);

  ut_ad(mutex_own(clone_sys->get_mutex()));
  mutex_exit(clone_sys->get_mutex());

  auto wait_restart = m_clone_task_manager.drop_task(thd, task_id, is_master);
  mutex_enter(clone_sys->get_mutex());

  /* Need to wait for restart, if network error */
  if (is_copy_clone() && m_allow_restart && wait_restart) {
    ut_ad(is_master);
    return (true);
  }

  /* Delete clone in progress file if successful. */
  if (!is_copy_clone() && is_master && in_err == 0) {
    delete_clone_file();
  }
  return (false);
}

int Clone_Handle::move_to_next_state(Clone_Task *task, Ha_clone_cbk *callback,
                                     Clone_Desc_State *state_desc) {
  auto snapshot = m_clone_task_manager.get_snapshot();
  /* Use input state only for apply. */
  auto next_state =
      is_copy_clone() ? snapshot->get_next_state() : state_desc->m_state;

  /* Move to new state */
  uint num_wait = 0;
  auto err =
      m_clone_task_manager.change_state(task, state_desc, next_state, num_wait);

  /* Need to wait for all other tasks to move over, if any. */
  if (num_wait > 0) {
    bool is_timeout = false;
    int alert_count = 0;
    err = Clone_Sys::wait_default(
        [&](bool alert, bool &result) {

          /* For multi threaded clone, master task does the state change. */
          if (task->m_is_master) {
            err = m_clone_task_manager.change_state(task, state_desc,
                                                    next_state, num_wait);
          } else {
            err = m_clone_task_manager.check_state(task, next_state, false, 0,
                                                   num_wait);
          }
          result = (num_wait > 0);

          if (err == 0 && result && alert) {
            /* Print messages every 1 minute - default is 5 seconds. */
            if (++alert_count == 12) {
              alert_count = 0;
              ib::info(ER_IB_MSG_151) << "Clone: master state change "
                                         "waiting for workers";
            }
            if (is_copy_clone()) {
              err = send_keep_alive(task, callback);
            }
          }
          return (err);
        },
        nullptr, is_timeout);

    if (err == 0 && !is_timeout) {
      return (0);
    }

    if (!task->m_is_master) {
      /* Exit from state transition */
      err = m_clone_task_manager.check_state(task, next_state, is_timeout, err,
                                             num_wait);
      if (err != 0 || num_wait == 0) {
        return (err);
      }
    }

    if (err == 0 && is_timeout) {
      ut_ad(false);
      ib::info(ER_IB_MSG_151) << "Clone: state change: "
                                 "wait for other tasks timed out";

      my_error(ER_INTERNAL_ERROR, MYF(0),
               "Clone: state change wait for other tasks timed out: "
               "Wait too long for state transition");
      return (ER_INTERNAL_ERROR);
    }
  }
  return (err);
}

int Clone_Handle::open_file(Clone_Task *task, Clone_File_Meta *file_meta,
                            ulint file_type, bool create_file,
                            bool set_and_close) {
  pfs_os_file_t handle;
  os_file_type_t type;
  ulint option;

  bool success;
  bool exists;
  bool read_only;

  /* Check if file exists */
  auto status = os_file_status(file_meta->m_file_name, &exists, &type);
  if (!status) {
    return (0);
  }

  if (create_file) {
    option = exists ? OS_FILE_OPEN : OS_FILE_CREATE_PATH;
    read_only = false;
  } else {
    ut_ad(exists);
    option = OS_FILE_OPEN;
    read_only = true;
  }

  option |= OS_FILE_ON_ERROR_NO_EXIT;

  handle = os_file_create(innodb_clone_file_key, file_meta->m_file_name, option,
                          OS_FILE_NORMAL, file_type, read_only, &success);

  if (success && set_and_close) {
    ut_ad(create_file);

    os_file_close(handle);

    if (success) {
      return (0);
    }
  }

  if (!success) {
    char errbuf[MYSYS_STRERROR_SIZE];

    int err =
        (option == OS_FILE_OPEN) ? ER_CANT_OPEN_FILE : ER_CANT_CREATE_FILE;

    my_error(err, MYF(0), file_meta->m_file_name, errno,
             my_strerror(errbuf, sizeof(errbuf), errno));

    return (err);
  }

  if (task == nullptr) {
    os_file_close(handle);
    return (0);
  }

  /* Set file descriptor in task. */
  close_file(task);
  task->m_current_file_des = handle;

  ut_ad(handle.m_file != OS_FILE_CLOSED);

  task->m_file_cache = true;

  /* Set cache to false if direct IO(O_DIRECT) is used. */
  if (file_type == OS_CLONE_DATA_FILE) {
    task->m_file_cache = !srv_is_direct_io();

    DBUG_EXECUTE_IF("clone_no_zero_copy", task->m_file_cache = false;);
  }

  task->m_current_file_index = file_meta->m_file_index;

  return (0);
}

int Clone_Handle::close_file(Clone_Task *task) {
  bool success = true;

  /* Close file, if opened. */
  if (task->m_current_file_des.m_file != OS_FILE_CLOSED) {
    success = os_file_close(task->m_current_file_des);
  }

  task->m_current_file_des.m_file = OS_FILE_CLOSED;
  task->m_current_file_index = 0;
  task->m_file_cache = true;

  if (!success) {
    my_error(ER_INTERNAL_ERROR, MYF(0), "Innodb error while closing file");
    return (ER_INTERNAL_ERROR);
  }

  return (0);
}

int Clone_Handle::file_callback(Ha_clone_cbk *cbk, Clone_Task *task, uint len
#ifdef UNIV_PFS_IO
                                ,
                                const char *name, uint line
#endif /* UNIV_PFS_IO */
) {
  int err;
  Ha_clone_file file;

  /* Platform specific code to set file handle */
#ifdef _WIN32
  file.type = Ha_clone_file::FILE_HANDLE;
  file.file_handle = static_cast<void *>(task->m_current_file_des.m_file);
#else
  file.type = Ha_clone_file::FILE_DESC;
  file.file_desc = task->m_current_file_des.m_file;
#endif /* _WIN32 */

  /* Register for PFS IO */
#ifdef UNIV_PFS_IO
  PSI_file_locker_state state;
  struct PSI_file_locker *locker;
  enum PSI_file_operation psi_op;

  locker = nullptr;
  psi_op = is_copy_clone() ? PSI_FILE_READ : PSI_FILE_WRITE;

  register_pfs_file_io_begin(&state, locker, task->m_current_file_des, len,
                             psi_op, name, line);
#endif /* UNIV_PFS_IO */

  /* Call appropriate callback to transfer data. */
  if (is_copy_clone()) {
    err = cbk->file_cbk(file, len);
  } else {
    err = cbk->apply_file_cbk(file);
  }

#ifdef UNIV_PFS_IO
  register_pfs_file_io_end(locker, len);
#endif /* UNIV_PFS_IO */

  return (err);
}

int Clone_Handle::send_task_metadata(Clone_Task *task, Ha_clone_cbk *callback) {
  Clone_Desc_Task_Meta task_desc;

  ut_ad(m_clone_handle_type == CLONE_HDL_COPY);

  /* Build task descriptor with metadata */
  task_desc.init_header(get_version());
  task_desc.m_task_meta = task->m_task_meta;

  auto desc_len = task->m_alloc_len;
  task_desc.serialize(task->m_serial_desc, desc_len, nullptr);

  callback->set_data_desc(task->m_serial_desc, desc_len);
  callback->clear_flags();
  callback->set_ack();

  auto err = callback->buffer_cbk(nullptr, 0);

  return (err);
}
int Clone_Handle::send_keep_alive(Clone_Task *task, Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_COPY);

  Clone_Desc_State state_desc;
  state_desc.init_header(get_version());

  /* Build state descriptor from snapshot and task */
  auto snapshot = m_clone_task_manager.get_snapshot();
  snapshot->get_state_info(false, &state_desc);

  state_desc.m_is_ack = true;

  auto task_meta = &task->m_task_meta;
  state_desc.m_task_index = task_meta->m_task_index;

  auto desc_len = task->m_alloc_len;
  state_desc.serialize(task->m_serial_desc, desc_len, nullptr);

  callback->set_data_desc(task->m_serial_desc, desc_len);
  callback->clear_flags();

  auto err = callback->buffer_cbk(nullptr, 0);

  return (err);
}

int Clone_Handle::send_state_metadata(Clone_Task *task, Ha_clone_cbk *callback,
                                      bool is_start) {
  ut_ad(m_clone_handle_type == CLONE_HDL_COPY);

  Clone_Desc_State state_desc;
  state_desc.init_header(get_version());

  /* Build state descriptor from snapshot and task */
  auto snapshot = m_clone_task_manager.get_snapshot();

  /* Master needs to send estimation while beginning state */
  auto get_estimate = (task->m_is_master && is_start);

  snapshot->get_state_info(get_estimate, &state_desc);

  /* Indicate if it is the end of state */
  state_desc.m_is_start = is_start;

  /* Check if remote has already acknowledged state transfer */
  if (!is_start && task->m_is_master &&
      !m_clone_task_manager.check_ack(&state_desc)) {
    ut_ad(task->m_is_master);
    ut_ad(m_clone_task_manager.is_restarted());

    ib::info(ER_IB_MSG_151) << "CLONE COPY: Skip ACK after restart for state "
                            << state_desc.m_state;
    return (0);
  }

  auto task_meta = &task->m_task_meta;
  state_desc.m_task_index = task_meta->m_task_index;

  auto desc_len = task->m_alloc_len;
  state_desc.serialize(task->m_serial_desc, desc_len, nullptr);

  callback->set_data_desc(task->m_serial_desc, desc_len);
  callback->clear_flags();
  callback->set_ack();

  auto err = callback->buffer_cbk(nullptr, 0);

  if (err != 0) {
    return (err);
  }

  if (is_start) {
    /* Send all file metadata while starting state */
    err = send_all_file_metadata(task, callback);

  } else {
    /* Wait for ACK while finishing state */
    err = m_clone_task_manager.wait_ack(this, task, callback);
  }

  return (err);
}


int Clone_Handle::send_all_file_metadata(Clone_Task *task,
                                         Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_COPY);

  if (!task->m_is_master) {
    return (0);
  }

  auto snapshot = m_clone_task_manager.get_snapshot();

  /* Send all file metadata for data/redo files */
  auto err = snapshot->iterate_files([&](Clone_File_Meta *file_meta) {
    return (send_file_metadata(task, file_meta, callback));
  });

  return (err);
}

int Clone_Handle::send_file_metadata(Clone_Task *task,
                                     Clone_File_Meta *file_meta,
                                     Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_COPY);

  auto snapshot = m_clone_task_manager.get_snapshot();

  Clone_Desc_File_MetaData file_desc;

  file_desc.m_file_meta = *file_meta;
  file_desc.m_state = snapshot->get_state();

  if (file_desc.m_state == CLONE_SNAPSHOT_REDO_COPY) {
    /* For Redo log always send the fixed redo file size. */
    file_desc.m_file_meta.m_file_size = snapshot->get_redo_file_size();

    file_desc.m_file_meta.m_file_name = nullptr;
    file_desc.m_file_meta.m_file_name_len = 0;

  } else if (file_meta->m_space_id == dict_sys_t_s_invalid_space_id) {
    /* Server buffer dump file ib_buffer_pool. */
    ut_ad(file_desc.m_state == CLONE_SNAPSHOT_FILE_COPY);
    ut_ad(file_meta->m_file_index == 0);

    file_desc.m_file_meta.m_file_name = SRV_BUF_DUMP_FILENAME_DEFAULT;

    file_desc.m_file_meta.m_file_name_len =
        static_cast<uint32_t>(strlen(SRV_BUF_DUMP_FILENAME_DEFAULT)) + 1;

  } else if (!fsp_is_ibd_tablespace(
                 static_cast<space_id_t>(file_meta->m_space_id)) &&
             Fil_path::is_absolute_path(file_meta->m_file_name)) {
    /* For system tablespace, remove absolute path. */
    ut_ad(file_desc.m_state == CLONE_SNAPSHOT_FILE_COPY);

    const char *name_ptr;

    name_ptr = strrchr(file_meta->m_file_name, OS_PATH_SEPARATOR);
    name_ptr++;

    file_desc.m_file_meta.m_file_name = name_ptr;
    file_desc.m_file_meta.m_file_name_len =
        static_cast<uint32_t>(strlen(name_ptr)) + 1;
  }

  file_desc.init_header(get_version());

  auto desc_len = task->m_alloc_len;
  file_desc.serialize(task->m_serial_desc, desc_len, nullptr);

  callback->set_data_desc(task->m_serial_desc, desc_len);
  callback->clear_flags();

  auto err = callback->buffer_cbk(nullptr, 0);

  return (err);
}


int Clone_Handle::send_data(Clone_Task *task, Clone_File_Meta *file_meta,
                            uint64_t offset, byte *buffer, uint size,
                            Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_COPY);

  auto snapshot = m_clone_task_manager.get_snapshot();

  /* Build data descriptor */
  Clone_Desc_Data data_desc;
  data_desc.init_header(get_version());
  data_desc.m_state = snapshot->get_state();

  data_desc.m_task_meta = task->m_task_meta;

  data_desc.m_file_index = file_meta->m_file_index;
  data_desc.m_data_len = size;
  data_desc.m_file_offset = offset;
  data_desc.m_file_size = file_meta->m_file_size;

  /* Serialize data descriptor and set in callback */
  auto desc_len = task->m_alloc_len;
  data_desc.serialize(task->m_serial_desc, desc_len, nullptr);

  callback->set_data_desc(task->m_serial_desc, desc_len);
  callback->clear_flags();

  auto file_type = OS_CLONE_DATA_FILE;
  bool is_log_file = (data_desc.m_state == CLONE_SNAPSHOT_REDO_COPY);

  if (is_log_file || file_meta->m_space_id == dict_sys_t_s_invalid_space_id) {
    file_type = OS_CLONE_LOG_FILE;
  }

  int err = 0;

  if (buffer != nullptr) {
    /* Send data from buffer. */
    err = callback->buffer_cbk(buffer, size);

  } else {
    /* Send data from file. */
    if (task->m_current_file_des.m_file == OS_FILE_CLOSED) {
      err = open_file(task, file_meta, file_type, false, false);

      if (err != 0) {
        return (err);
      }
    }

    ut_ad(task->m_current_file_index == file_meta->m_file_index);

    os_file_t file_hdl;
    char errbuf[MYSYS_STRERROR_SIZE];

    file_hdl = task->m_current_file_des.m_file;
    auto success = os_file_seek(nullptr, file_hdl, offset);

    if (!success) {
      my_error(ER_ERROR_ON_READ, MYF(0), file_meta->m_file_name, errno,
               my_strerror(errbuf, sizeof(errbuf), errno));
      return (ER_ERROR_ON_READ);
    }

    if (task->m_file_cache) {
      callback->set_os_buffer_cache();
      /* For data file recommend zero copy for cached IO. */
      if (!is_log_file) {
        callback->set_zero_copy();
      }
    }

    callback->set_source_name(file_meta->m_file_name);

    err = file_callback(callback, task, size
#ifdef UNIV_PFS_IO
                        ,
                        __FILE__, __LINE__
#endif /* UNIV_PFS_IO */
    );
  }

  task->m_data_size += size;

  return (err);
}

void Clone_Handle::display_progress(uint32_t cur_chunk, uint32_t max_chunk,
                                    uint32_t &percent_done, ulint &disp_time) {
  auto current_time = ut_time_ms();
  auto current_percent = (cur_chunk * 100) / max_chunk;

  if (current_percent >= percent_done + 20 ||
      (current_time - disp_time > 5000 && current_percent > percent_done)) {
    percent_done = current_percent;
    disp_time = current_time;

    ib::info(ER_IB_MSG_154)
        << "Stage progress: " << percent_done << "% completed.";
  }
}


int Clone_Handle::copy(THD *thd, uint task_id, Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_COPY);

  /* Get task from task manager. */
  auto task = m_clone_task_manager.get_task_by_index(task_id);

  auto err = m_clone_task_manager.alloc_buffer(task);
  if (err != 0) {
    return (err);
  }

  /* Allow restart only after copy is started. */
  m_allow_restart = true;

  /* Send the task metadata. */
  err = send_task_metadata(task, callback);

  if (err != 0) {
    return (err);
  }

  auto send_matadata = m_clone_task_manager.is_restart_metadata(task);

  /* Send state metadata to remote during restart */
  if (send_matadata) {
    ut_ad(task->m_is_master);
    ut_ad(m_clone_task_manager.is_restarted());

    err = send_state_metadata(task, callback, true);

    /* Send all file metadata during restart */
  } else if (task->m_is_master &&
             m_clone_task_manager.get_state() == CLONE_SNAPSHOT_FILE_COPY &&
             !m_clone_task_manager.is_file_metadata_transferred()) {
    ut_ad(m_clone_task_manager.is_restarted());
    err = send_all_file_metadata(task, callback);
  }

  if (err != 0) {
    return (err);
  }
  /* Adjust block size based on client buffer size. */
  auto snapshot = m_clone_task_manager.get_snapshot();
  snapshot->update_block_size(callback->get_client_buffer_size());

  auto max_chunks = snapshot->get_num_chunks();

  /* Set time values for tracking stage progress. */

  auto disp_time = ut_time_ms();

  /* Loop and process data until snapshot is moved to DONE state. */
  uint32_t percent_done = 0;

  while (m_clone_task_manager.get_state() != CLONE_SNAPSHOT_DONE) {
    /* Reserve next chunk for current state from snapshot. */
    uint32_t current_chunk = 0;
    uint32_t current_block = 0;

    err = m_clone_task_manager.reserve_next_chunk(task, current_chunk,
                                                  current_block);

    if (err != 0) {
      break;
    }

    if (current_chunk != 0) {
      /* Send blocks from the reserved chunk. */
      err = process_chunk(task, current_chunk, current_block, callback);

      /* Display stage progress based on % completion. */
      if (task->m_is_master) {
        display_progress(current_chunk, max_chunks, percent_done, disp_time);
      }

    } else {
      /* No more chunks in current state. Transit to next state. */

      /* Close the last open file before proceeding to next state */
      err = close_file(task);

      if (err != 0) {
        break;
      }

      /* Inform that the data transfer for current state
      is over before moving to next state. The remote
      needs to send back state transfer ACK for the state
      transfer to complete. */
      err = send_state_metadata(task, callback, false);

      if (err != 0) {
        break;
      }

      /* Next state is decided by snapshot for Copy. */
      err = move_to_next_state(task, callback, nullptr);

      ut_d(task->m_ignore_sync = false);

      if (err != 0) {
        break;
      }

      max_chunks = snapshot->get_num_chunks();
      percent_done = 0;
      disp_time = ut_time_ms();

      /* Send state metadata before processing chunks. */
      err = send_state_metadata(task, callback, true);
    }

    if (err != 0) {
      break;
    }
  }

  /* Close the last open file after error. */
  if (err != 0) {
    close_file(task);
  }

  return (err);
}


int Clone_Handle::process_chunk(Clone_Task *task, uint32_t chunk_num,
                                uint32_t block_num, Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_COPY);

  Clone_File_Meta file_meta;

  auto &task_meta = task->m_task_meta;

  file_meta.m_file_index = 0;

  /* If chunks are in increasing order, optimize file
  search by index */
  if (task_meta.m_chunk_num <= chunk_num) {
    file_meta.m_file_index = task->m_current_file_index;
  }

  auto snapshot = m_clone_task_manager.get_snapshot();
  auto state = m_clone_task_manager.get_state();

  /* Loop over all the blocks of current chunk and send data. */
  int err = 0;

  while (err == 0) {
    auto data_buf = task->m_current_buffer;
    auto data_size = task->m_buffer_alloc_len;

    /* Get next block from snapshot */
    ib_uint64_t data_offset;

    err = snapshot->get_next_block(chunk_num, block_num, &file_meta,
                                   data_offset, data_buf, data_size);

    /* '0' block number indicates no more blocks. */
    if (err != 0 || block_num == 0) {
      break;
    }

    /* Check for error from other tasks and DDL */
    err = m_clone_task_manager.handle_error_other_task(task->m_has_thd);

    if (err != 0) {
      break;
    }

    task->m_task_meta.m_block_num = block_num;
    task->m_task_meta.m_chunk_num = chunk_num;

    if (state != CLONE_SNAPSHOT_PAGE_COPY &&
        (task->m_current_file_des.m_file == OS_FILE_CLOSED ||
         task->m_current_file_index != file_meta.m_file_index)) {
      /* We are moving to next file. Close the current file and
      send metadata for the next file. */

      err = close_file(task);

      if (err != 0) {
        break;
      }

      /* During redo copy, worker could be ahead of master and needs to
      send the metadata */
      if (state == CLONE_SNAPSHOT_REDO_COPY) {
        err = send_file_metadata(task, &file_meta, callback);
      }

      if (err != 0) {
        break;
      }
    }

    if (data_size == 0) {
      continue;
    }

    err =
        send_data(task, &file_meta, data_offset, data_buf, data_size, callback);
  }

  /* Save current error and file name. */
  if (err != 0) {
    m_clone_task_manager.set_error(err, file_meta.m_file_name);
  }

  return (err);
}



int Clone_Handle::restart_copy(THD *thd, const byte *loc, uint loc_len) {
  ut_ad(mutex_own(clone_sys->get_mutex()));

  if (is_abort()) {
    my_error(ER_INTERNAL_ERROR, MYF(0),
             "Innodb Clone Restart failed, existing clone aborted");
    return (ER_INTERNAL_ERROR);
  }

  /* Wait for the Idle state */
  if (!is_idle()) {
    /* Sleep for 1 second */
    Clone_Msec sleep_time(Clone_Sec(1));
    /* Generate alert message every 5 seconds. */
    Clone_Sec alert_time(5);
    /* Wait for 30 seconds for server to reach idle state. */
    Clone_Sec time_out(30);

    bool is_timeout = false;
    auto err = Clone_Sys::wait(
        sleep_time, time_out, alert_time,
        [&](bool alert, bool &result) {
          ut_ad(mutex_own(clone_sys->get_mutex()));
          result = !is_idle();

          if (thd_killed(thd)) {
            my_error(ER_QUERY_INTERRUPTED, MYF(0));
            return (ER_QUERY_INTERRUPTED);

          } else if (is_abort()) {
            my_error(ER_INTERNAL_ERROR, MYF(0),
                     "Innodb Clone Restart failed, existing clone aborted");
            return (ER_INTERNAL_ERROR);

          } else if (Clone_Sys::s_clone_sys_state == CLONE_SYS_ABORT) {
            my_error(ER_DDL_IN_PROGRESS, MYF(0));
            return (ER_DDL_IN_PROGRESS);
          }

          if (result && alert) {
            ib::info(ER_IB_MSG_151) << "Clone Master Restart "
                                       "wait for idle state";
          }
          return (0);
        },
        clone_sys->get_mutex(), is_timeout);

    if (err != 0) {
      return (err);

    } else if (is_timeout) {
      ib::info(ER_IB_MSG_151) << "Clone Master restart wait for idle timed out";

      my_error(ER_INTERNAL_ERROR, MYF(0),
               "Clone restart wait for idle state timed out");
      return (ER_INTERNAL_ERROR);
    }
  }

  ut_ad(is_idle());
  m_clone_task_manager.reinit_copy_state(loc, loc_len);

  set_state(CLONE_STATE_ACTIVE);

  return (0);
}


bool Clone_Snapshot::add_file_from_desc(Clone_File_Meta *&file_desc) {
  mutex_enter(&m_snapshot_mutex);

  ut_ad(m_snapshot_handle_type == CLONE_HDL_APPLY);

  if (m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY) {
    m_data_file_vector[file_desc->m_file_index] = file_desc;
  } else {
    ut_ad(m_snapshot_state == CLONE_SNAPSHOT_REDO_COPY);
    m_redo_file_vector[file_desc->m_file_index] = file_desc;
  }

  mutex_exit(&m_snapshot_mutex);

  /** Check if it the last file */
  if (file_desc->m_file_index == m_num_data_files - 1) {
    return true;
  }

  return (false);
}

int Clone_Handle::apply_task_metadata(Clone_Task *task,
                                      Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_APPLY);
  uint desc_len = 0;
  auto serial_desc = callback->get_data_desc(&desc_len);

  Clone_Desc_Task_Meta task_desc;
  auto success = task_desc.deserialize(serial_desc, desc_len);

  if (!success) {
    ut_ad(false);
    int err = ER_CLONE_PROTOCOL_ERROR;
    my_error(err, MYF(0), "Wrong Clone RPC: Invalid Task Descriptor");
    return (err);
  }
  task->m_task_meta = task_desc.m_task_meta;
  return (0);
}

int Clone_Handle::apply_state_metadata(Clone_Task *task,
                                       Ha_clone_cbk *callback) {
  int err = 0;
  uint desc_len = 0;
  auto serial_desc = callback->get_data_desc(&desc_len);

  Clone_Desc_State state_desc;
  auto success = state_desc.deserialize(serial_desc, desc_len);

  if (!success) {
    ut_ad(false);
    err = ER_CLONE_PROTOCOL_ERROR;
    my_error(err, MYF(0), "Wrong Clone RPC: Invalid State Descriptor");
    return (err);
  }
  if (m_clone_handle_type == CLONE_HDL_COPY) {
    ut_ad(state_desc.m_is_ack);
    m_clone_task_manager.ack_state(&state_desc);
    return (0);
  }

  ut_ad(m_clone_handle_type == CLONE_HDL_APPLY);

  /* ACK descriptor is sent for keeping the connection alive. */
  if (state_desc.m_is_ack) {
    return (0);
  }

  /* Reset current chunk information */
  auto &task_meta = task->m_task_meta;
  task_meta.m_chunk_num = 0;
  task_meta.m_block_num = 0;

  /* Move to the new state */
  if (state_desc.m_is_start) {
#ifdef UNIV_DEBUG
    /* Network failure before moving to new state */
    err = m_clone_task_manager.debug_restart(task, err, 5);
#endif /* UNIV_DEBUG */

    err = move_to_next_state(task, nullptr, &state_desc);

#ifdef UNIV_DEBUG
    /* Network failure after moving to new state */
    err = m_clone_task_manager.debug_restart(task, err, 0);
#endif /* UNIV_DEBUG */

    return (err);
  }

  /* It is the end of current state. Close active file. */
  err = close_file(task);

#ifdef UNIV_DEBUG
  /* Network failure before finishing state */
  err = m_clone_task_manager.debug_restart(task, err, 2);
#endif /* UNIV_DEBUG */

  if (err != 0) {
    return (err);
  }

  ut_ad(state_desc.m_state == m_clone_task_manager.get_state());

  /* Mark current state finished for the task */
  err = m_clone_task_manager.finish_state(task);

#ifdef UNIV_DEBUG
  /* Network failure before sending ACK */
  err = m_clone_task_manager.debug_restart(task, err, 3);
#endif /* UNIV_DEBUG */

  /* Send acknowledgement back to remote server */
  if (err == 0 && task->m_is_master) {
    err = ack_state_metadata(task, callback, &state_desc);

    if (err != 0) {
      ib::info(ER_IB_MSG_151)
          << "Clone Apply Master ACK finshed state: " << state_desc.m_state;
    }
  }

#ifdef UNIV_DEBUG
  /* Network failure after sending ACK */
  err = m_clone_task_manager.debug_restart(task, err, 4);
#endif /* UNIV_DEBUG */

  return (err);
}


int Clone_Handle::ack_state_metadata(Clone_Task *task, Ha_clone_cbk *callback,
                                     Clone_Desc_State *state_desc) {
  ut_ad(m_clone_handle_type == CLONE_HDL_APPLY);

  state_desc->m_is_ack = true;

  byte desc_buf[CLONE_DESC_MAX_BASE_LEN];

  auto serial_desc = &desc_buf[0];
  uint desc_len = CLONE_DESC_MAX_BASE_LEN;

  state_desc->serialize(serial_desc, desc_len, nullptr);

  callback->set_data_desc(serial_desc, desc_len);
  callback->clear_flags();

  auto err = callback->buffer_cbk(nullptr, 0);

  return (err);
}

int Clone_Handle::apply_file_metadata(Clone_Task *task,
                                      Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_APPLY);

  uint desc_len = 0;
  auto serial_desc = callback->get_data_desc(&desc_len);

  Clone_Desc_File_MetaData file_desc;
  auto success = file_desc.deserialize(serial_desc, desc_len);

  if (!success) {
    ut_ad(false);
    int err = ER_CLONE_PROTOCOL_ERROR;
    my_error(err, MYF(0), "Wrong Clone RPC: Invalid File Descriptor");
    return (err);
  }
  auto file_meta = &file_desc.m_file_meta;
  auto snapshot = m_clone_task_manager.get_snapshot();

  ut_ad(snapshot->get_state() == file_desc.m_state);

  bool desc_exists;

  /* Check file metadata entry based on the descriptor. */
  auto err =
      snapshot->get_file_from_desc(file_meta, m_clone_dir, false, desc_exists);
  if (err != 0 || desc_exists) {
    return (err);
  }

  mutex_enter(m_clone_task_manager.get_mutex());

  /* Create file metadata entry based on the descriptor. */
  err = snapshot->get_file_from_desc(file_meta, m_clone_dir, true, desc_exists);

  if (err != 0 || desc_exists) {
    mutex_exit(m_clone_task_manager.get_mutex());

    /* Save error with file name. */
    if (err != 0) {
      m_clone_task_manager.set_error(err, file_meta->m_file_name);
    }
    return (err);
  }

  if (file_desc.m_state == CLONE_SNAPSHOT_FILE_COPY) {
    auto file_type = OS_CLONE_DATA_FILE;

    if (file_meta->m_space_id == dict_sys_t_s_invalid_space_id) {
      file_type = OS_CLONE_LOG_FILE;
    }

    /* Create the file */
    err = open_file(nullptr, file_meta, file_type, true, false);

    /* If last file is received, set all file metadata transferred */
    if (snapshot->add_file_from_desc(file_meta)) {
      m_clone_task_manager.set_file_meta_transferred();
    }

    mutex_exit(m_clone_task_manager.get_mutex());
    return (err);
  }

  ut_ad(file_desc.m_state == CLONE_SNAPSHOT_REDO_COPY);

  /* open and reserve the redo file size */
  err = open_file(nullptr, file_meta, OS_CLONE_LOG_FILE, true, true);

  snapshot->add_file_from_desc(file_meta);

  /* For redo copy, check and add entry for the second file. */
  if (err == 0 && file_meta->m_file_index == 0) {
    file_meta = &file_desc.m_file_meta;
    file_meta->m_file_index++;

    err =
        snapshot->get_file_from_desc(file_meta, m_clone_dir, true, desc_exists);

    if (err == 0 && !desc_exists) {
      err = open_file(nullptr, file_meta, OS_CLONE_LOG_FILE, true, true);
      snapshot->add_file_from_desc(file_meta);
    }
  }

  mutex_exit(m_clone_task_manager.get_mutex());
  return (err);
}

int Clone_Handle::receive_data(Clone_Task *task, uint64_t offset,
                               uint64_t file_size, uint32_t size,
                               Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_APPLY);

  auto snapshot = m_clone_task_manager.get_snapshot();

  auto file_meta = snapshot->get_file_by_index(task->m_current_file_index);

  /* Check and update file size for space header page */
  if (snapshot->get_state() == CLONE_SNAPSHOT_PAGE_COPY && offset == 0 &&
      file_meta->m_file_size < file_size) {
    snapshot->update_file_size(task->m_current_file_index, file_size);
  }

  auto file_type = OS_CLONE_DATA_FILE;
  bool is_log_file = (snapshot->get_state() == CLONE_SNAPSHOT_REDO_COPY);

  if (is_log_file || file_meta->m_space_id == dict_sys_t_s_invalid_space_id) {
    file_type = OS_CLONE_LOG_FILE;
  }

  /* Open destination file for first block. */
  if (task->m_current_file_des.m_file == OS_FILE_CLOSED) {
    ut_ad(file_meta != nullptr);

    auto err = open_file(task, file_meta, file_type, true, false);

    if (err != 0) {
      /* Save error with file name. */
      m_clone_task_manager.set_error(err, file_meta->m_file_name);
      return (err);
    }
  }

  ut_ad(task->m_current_file_index == file_meta->m_file_index);

  /* Copy data to current destination file using callback. */
  char errbuf[MYSYS_STRERROR_SIZE];

  auto file_hdl = task->m_current_file_des.m_file;
  auto success = os_file_seek(nullptr, file_hdl, offset);

  if (!success) {
    my_error(ER_ERROR_ON_READ, MYF(0), file_meta->m_file_name, errno,
             my_strerror(errbuf, sizeof(errbuf), errno));
    /* Save error with file name. */
    m_clone_task_manager.set_error(ER_ERROR_ON_READ, file_meta->m_file_name);
    return (ER_ERROR_ON_READ);
  }

  if (task->m_file_cache) {
    callback->set_os_buffer_cache();
    /* For data file recommend zero copy for cached IO. */
    if (!is_log_file) {
      callback->set_zero_copy();
    }
  }

  callback->set_dest_name(file_meta->m_file_name);

  auto err = file_callback(callback, task, size
#ifdef UNIV_PFS_IO
                           ,
                           __FILE__, __LINE__
#endif /* UNIV_PFS_IO */
  );

  task->m_data_size += size;

  if (err != 0) {
    /* Save error with file name. */
    m_clone_task_manager.set_error(err, file_meta->m_file_name);
  }
  return (err);
}

int Clone_Handle::apply_data(Clone_Task *task, Ha_clone_cbk *callback) {
  ut_ad(m_clone_handle_type == CLONE_HDL_APPLY);

  /* Extract the data descriptor. */
  uint desc_len = 0;
  auto serial_desc = callback->get_data_desc(&desc_len);

  Clone_Desc_Data data_desc;
  auto success = data_desc.deserialize(serial_desc, desc_len);

  if (!success) {
    ut_ad(false);
    int err = ER_CLONE_PROTOCOL_ERROR;
    my_error(err, MYF(0), "Wrong Clone RPC: Invalid Data Descriptor");
    return (err);
  }
  /* Identify the task for the current block of data. */
  int err = 0;
  auto task_meta = &data_desc.m_task_meta;

  /* The data is from a different file. Close the current one. */
  if (task->m_current_file_index != data_desc.m_file_index) {
    err = close_file(task);
    if (err != 0) {
      return (err);
    }
    task->m_current_file_index = data_desc.m_file_index;
  }

  /* Receive data from callback and apply. */
  err = receive_data(task, data_desc.m_file_offset, data_desc.m_file_size,
                     data_desc.m_data_len, callback);

  /* Close file in case of error. */
  if (err != 0) {
    close_file(task);
  } else {
    err = m_clone_task_manager.set_chunk(task, task_meta);
  }

  return (err);
}


int Clone_Handle::apply(THD *thd, uint task_id, Ha_clone_cbk *callback) {
  int err = 0;
  uint desc_len = 0;

  auto clone_desc = callback->get_data_desc(&desc_len);
  ut_ad(clone_desc != nullptr);

  Clone_Desc_Header header;
  auto success = header.deserialize(clone_desc, desc_len);

  if (!success) {
    ut_ad(false);
    err = ER_CLONE_PROTOCOL_ERROR;
    my_error(err, MYF(0), "Wrong Clone RPC: Invalid Descriptor Header");
    return (err);
  }

  /* Check the descriptor type in header and apply */
  auto task = m_clone_task_manager.get_task_by_index(task_id);

  switch (header.m_type) {
    case CLONE_DESC_TASK_METADATA:
      err = apply_task_metadata(task, callback);
      break;

    case CLONE_DESC_STATE:
      err = apply_state_metadata(task, callback);
      break;

    case CLONE_DESC_FILE_METADATA:
      err = apply_file_metadata(task, callback);
      break;

    case CLONE_DESC_DATA:
      err = apply_data(task, callback);
      break;

    default:
      ut_ad(false);
      break;
  }

  if (err != 0) {
    close_file(task);
  }

  return (err);
}

int Clone_Handle::restart_apply(THD *thd, const byte *&loc, uint &loc_len) {
  auto init_loc = m_restart_loc;
  auto init_len = m_restart_loc_len;
  auto alloc_len = m_restart_loc_len;

  /* Get latest locator */
  loc = get_locator(loc_len);

  m_clone_task_manager.reinit_apply_state(loc, loc_len, init_loc, init_len,
                                          alloc_len);

  /* Return the original locator if no state information */
  if (init_loc == nullptr) {
    return (0);
  }

  loc = init_loc;
  loc_len = init_len;

  /* Reset restart loc buffer if newly allocated */
  if (alloc_len > m_restart_loc_len) {
    m_restart_loc = init_loc;
    m_restart_loc_len = alloc_len;
  }

  ut_ad(loc == m_restart_loc);

  auto master_task = m_clone_task_manager.get_task_by_index(0);

  auto err = close_file(master_task);

  return (err);
}

void Clone_Snapshot::update_file_size(uint32_t file_index, uint64_t file_size) {
  /* Update file size when file is extended during page copy */
  ut_ad(m_snapshot_state == CLONE_SNAPSHOT_PAGE_COPY);

  auto cur_file = get_file_by_index(file_index);

  while (file_size > cur_file->m_file_size) {
    ++file_index;

    if (file_index >= m_num_data_files) {
      /* Update file size for the last file. */
      cur_file->m_file_size = file_size;
      break;
    }

    auto next_file = get_file_by_index(file_index);

    if (next_file->m_space_id != cur_file->m_space_id) {
      /* Update file size for the last file. */
      cur_file->m_file_size = file_size;
      break;
    }

    /* Only system tablespace can have multiple nodes. */
    ut_ad(cur_file->m_space_id == 0);

    file_size -= cur_file->m_file_size;
    cur_file = next_file;
  }
}

int Clone_Snapshot::init_apply_state(Clone_Desc_State *state_desc) {
  set_state_info(state_desc);

  int err = 0;
  switch (m_snapshot_state) {
    case CLONE_SNAPSHOT_FILE_COPY:
      ib::info(ER_IB_MSG_151) << "Clone Apply State FILE COPY: ";
      break;

    case CLONE_SNAPSHOT_PAGE_COPY:
      ib::info(ER_IB_MSG_152) << "Clone Apply State PAGE COPY: ";
      break;

    case CLONE_SNAPSHOT_REDO_COPY:
      ib::info(ER_IB_MSG_152) << "Clone Apply State REDO COPY: ";
      break;

    case CLONE_SNAPSHOT_DONE:
      /* Extend and flush data files. */
      ib::info(ER_IB_MSG_153) << "Clone Apply State FLUSH DATA: ";
      err = extend_and_flush_files(false);
      if (err != 0) {
        ib::info(ER_IB_MSG_153)
            << "Clone Apply FLUSH DATA failed code: " << err;
        break;
      }
      /* Flush redo files. */
      ib::info(ER_IB_MSG_153) << "Clone Apply State FLUSH REDO: ";
      err = extend_and_flush_files(true);
      if (err != 0) {
        ib::info(ER_IB_MSG_153)
            << "Clone Apply FLUSH REDO failed code: " << err;
        break;
      }
      ib::info(ER_IB_MSG_154) << "Clone Apply State DONE";
      break;

    case CLONE_SNAPSHOT_NONE:
    case CLONE_SNAPSHOT_INIT:
    default:
      ut_ad(false);
      err = ER_INTERNAL_ERROR;
      my_error(err, MYF(0), "Innodb Clone Snapshot Invalid state");
      break;
  }
  return (err);
}

int Clone_Snapshot::extend_and_flush_files(bool flush_redo) {
  auto &file_vector = (flush_redo) ? m_redo_file_vector : m_data_file_vector;

  for (auto file_meta : file_vector) {
    char errbuf[MYSYS_STRERROR_SIZE];
    bool success = true;

    auto file =
        os_file_create(innodb_clone_file_key, file_meta->m_file_name,
                       OS_FILE_OPEN | OS_FILE_ON_ERROR_NO_EXIT, OS_FILE_NORMAL,
                       OS_CLONE_DATA_FILE, false, &success);

    if (!success) {
      my_error(ER_CANT_OPEN_FILE, MYF(0), file_meta->m_file_name, errno,
               my_strerror(errbuf, sizeof(errbuf), errno));

      return (ER_CANT_OPEN_FILE);
    }

    auto file_size = os_file_get_size(file);

    if (file_size < file_meta->m_file_size) {
      success = os_file_set_size(file_meta->m_file_name, file, file_size,
                                 file_meta->m_file_size, false, true);
    } else {
      success = os_file_flush(file);
    }

    os_file_close(file);

    if (!success) {
      my_error(ER_ERROR_ON_WRITE, MYF(0), file_meta->m_file_name, errno,
               my_strerror(errbuf, sizeof(errbuf), errno));

      return (ER_ERROR_ON_WRITE);
    }
  }
  return (0);
}

