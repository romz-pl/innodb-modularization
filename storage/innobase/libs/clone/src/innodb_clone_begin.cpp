#include <innodb/clone/innodb_clone_begin.h>

#include <innodb/clone/Clone_Handle.h>
#include <innodb/clone/clone_sys.h>
#include <innodb/clone/clone_validate_locator.h>
#include <innodb/clone/srv_redo_log_encrypt.h>
#include <innodb/clone/srv_undo_log_encrypt.h>
#include <innodb/sync_mutex/IB_mutex_guard.h>

int innodb_clone_begin(handlerton *hton, THD *thd, const byte *&loc,
                       uint &loc_len, uint &task_id, Ha_clone_type type,
                       Ha_clone_mode mode) {
  Clone_Handle *clone_hdl;

  /* Encrypted redo or undo log clone is not supported */
  if (srv_redo_log_encrypt || srv_undo_log_encrypt) {
    if (thd != nullptr) {
      my_error(ER_NOT_SUPPORTED_YET, MYF(0), "Clone Encrypted logs");
    }

    return (ER_NOT_SUPPORTED_YET);
  }

  /* Check if reference locator is valid */
  if (loc != nullptr && !clone_validate_locator(loc, loc_len)) {
    int err = ER_CLONE_PROTOCOL_ERROR;
    my_error(err, MYF(0), "Wrong Clone RPC: Invalid Locator");
    return (err);
  }

  /* Acquire clone system mutex which would automatically get released
  when we return from the function [RAII]. */
  IB_mutex_guard sys_mutex(clone_sys->get_mutex());

  /* Check if concurrent ddl has marked abort. */
  if (Clone_Sys::s_clone_sys_state == CLONE_SYS_ABORT) {
    if (thd != nullptr) {
      my_error(ER_DDL_IN_PROGRESS, MYF(0));
    }

    return (ER_DDL_IN_PROGRESS);
  }

  /* Check of clone is already in progress for the reference locator. */
  clone_hdl = clone_sys->find_clone(loc, loc_len, CLONE_HDL_COPY);

  int err = 0;

  switch (mode) {
    case HA_CLONE_MODE_RESTART:
      /* Error out if existing clone is not found */
      if (clone_hdl == nullptr) {
        my_error(ER_INTERNAL_ERROR, MYF(0),
                 "Innodb Clone Restart could not find existing clone");
        return (ER_INTERNAL_ERROR);
      }

      ib::info(ER_IB_MSG_151) << "Clone Begin Master Task: Restart";
      err = clone_hdl->restart_copy(thd, loc, loc_len);

      break;

    case HA_CLONE_MODE_START:
      /* Should not find existing clone for the locator */
      if (clone_hdl != nullptr) {
        clone_sys->drop_clone(clone_hdl);
        my_error(ER_INTERNAL_ERROR, MYF(0),
                 "Innodb Clone Begin refers existing clone");
        return (ER_INTERNAL_ERROR);
      }
      ib::info(ER_IB_MSG_151) << "Clone Begin Master Task";
      break;

    case HA_CLONE_MODE_ADD_TASK:
      /* Should find existing clone for the locator */
      if (clone_hdl == nullptr) {
        /* Operation has finished already */
        my_error(ER_INTERNAL_ERROR, MYF(0),
                 "Innodb Clone add task refers non-existing clone");

        return (ER_INTERNAL_ERROR);
      }
      break;

    case HA_CLONE_MODE_VERSION:
    case HA_CLONE_MODE_MAX:
    default:
      ut_ad(false);
      my_error(ER_INTERNAL_ERROR, MYF(0), "Innodb Clone Begin Invalid Mode");

      return (ER_INTERNAL_ERROR);
  }

  if (clone_hdl == nullptr) {
    ut_ad(thd != nullptr);
    ut_ad(mode == HA_CLONE_MODE_START);

    /* Create new clone handle for copy. Reference locator
    is used for matching the version. */
    auto err = clone_sys->add_clone(loc, CLONE_HDL_COPY, clone_hdl);
    if (err != 0) {
      return (err);
    }

    err = clone_hdl->init(loc, loc_len, type, nullptr);

    if (err != 0) {
      clone_sys->drop_clone(clone_hdl);
      return (err);
    }
  }

  /* Add new task for the clone copy operation. */
  if (err == 0) {
    /* Release clone system mutex here as we might need to wait while
    adding task. It is safe as the clone handle is acquired and cannot
    be freed till we release it. */
    mutex_exit(clone_sys->get_mutex());
    err = clone_hdl->add_task(thd, nullptr, 0, task_id);
    mutex_enter(clone_sys->get_mutex());
  }

  if (err != 0) {
    clone_sys->drop_clone(clone_hdl);
    return (err);
  }

  if (task_id > 0) {
    ib::info(ER_IB_MSG_151) << "Clone Begin Task ID: " << task_id;
  }

  /* Get the current locator from clone handle. */
  loc = clone_hdl->get_locator(loc_len);
  return (0);
}

