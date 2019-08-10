#include <innodb/clone/Clone_Handle.h>

#include <innodb/clone/clone_sys.h>
#include <innodb/clone/clone_sys.h>
#include <innodb/io/create_purpose.h>
#include <innodb/io/create_type.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_create.h>
#include <innodb/io/os_file_create_directory.h>
#include <innodb/io/os_file_create_subdirs_if_needed.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/os_file_delete.h>
#include <innodb/io/pfs.h>
#include <innodb/io/os_file_type_t.h>
#include <innodb/io/srv_is_direct_io.h>
#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/io/os_file_status.h>


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
