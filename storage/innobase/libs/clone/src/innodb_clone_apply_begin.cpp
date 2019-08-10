#include <innodb/clone/innodb_clone_apply_begin.h>

#include <innodb/sync_mutex/IB_mutex_guard.h>
#include <innodb/clone/clone_sys.h>
#include <innodb/clone/clone_validate_locator.h>

int innodb_clone_apply_begin(handlerton *hton, THD *thd, const byte *&loc,
                             uint &loc_len, uint &task_id, Ha_clone_mode mode,
                             const char *data_dir) {
  /* Check if reference locator is valid */
  if (loc != nullptr && !clone_validate_locator(loc, loc_len)) {
    int err = ER_CLONE_PROTOCOL_ERROR;
    my_error(err, MYF(0), "Wrong Clone RPC: Invalid Locator");
    return (err);
  }

  /* Acquire clone system mutex which would automatically get released
  when we return from the function [RAII]. */
  IB_mutex_guard sys_mutex(clone_sys->get_mutex());

  /* Check if clone is already in progress for the reference locator. */
  auto clone_hdl = clone_sys->find_clone(loc, loc_len, CLONE_HDL_APPLY);

  switch (mode) {
    case HA_CLONE_MODE_RESTART: {
      ib::info(ER_IB_MSG_151) << "Clone Apply Begin Master Task: Restart";
      auto err = clone_hdl->restart_apply(thd, loc, loc_len);

      /* Reduce reference count */
      clone_sys->drop_clone(clone_hdl);

      /* Restart is done by master task */
      ut_ad(task_id == 0);
      task_id = 0;

      return (err);
    }
    case HA_CLONE_MODE_START:

      ut_ad(clone_hdl == nullptr);
      ib::info(ER_IB_MSG_151) << "Clone Apply Begin Master Task";
      break;

    case HA_CLONE_MODE_ADD_TASK:
      /* Should find existing clone for the locator */
      if (clone_hdl == nullptr) {
        /* Operation has finished already */
        my_error(ER_INTERNAL_ERROR, MYF(0),
                 "Innodb Clone Apply add task to non-existing clone");

        return (ER_INTERNAL_ERROR);
      }
      break;

    case HA_CLONE_MODE_VERSION:
      /* Cannot have input locator or existing clone */
      ib::info(ER_IB_MSG_151) << "Clone Apply Begin Master Version Check";
      ut_ad(loc == nullptr);
      ut_ad(clone_hdl == nullptr);
      break;

    case HA_CLONE_MODE_MAX:
    default:
      ut_ad(false);

      my_error(ER_INTERNAL_ERROR, MYF(0),
               "Innodb Clone Appply Begin Invalid Mode");

      return (ER_INTERNAL_ERROR);
  }

  if (clone_hdl == nullptr) {
    ut_ad(thd != nullptr);

    ut_ad(mode == HA_CLONE_MODE_VERSION || mode == HA_CLONE_MODE_START);

    /* Create new clone handle for apply. Reference locator
    is used for matching the version. */
    auto err = clone_sys->add_clone(loc, CLONE_HDL_APPLY, clone_hdl);
    if (err != 0) {
      return (err);
    }

    err = clone_hdl->init(loc, loc_len, HA_CLONE_BLOCKING, data_dir);

    if (err != 0) {
      clone_sys->drop_clone(clone_hdl);
      return (err);
    }
  }

  if (clone_hdl->is_active()) {
    /* Release clone system mutex here as we might need to wait while
    adding task. It is safe as the clone handle is acquired and cannot
    be freed till we release it. */
    mutex_exit(clone_sys->get_mutex());
    /* Add new task for the clone apply operation. */
    ut_ad(loc != nullptr);
    auto err = clone_hdl->add_task(thd, loc, loc_len, task_id);
    mutex_enter(clone_sys->get_mutex());

    if (err != 0) {
      clone_sys->drop_clone(clone_hdl);
      return (err);
    }
  } else {
    ut_ad(mode == HA_CLONE_MODE_VERSION);
  }

  if (task_id > 0) {
    ib::info(ER_IB_MSG_151) << "Clone Apply Begin Task ID: " << task_id;
  }
  /* Get the current locator from clone handle. */
  if (mode != HA_CLONE_MODE_ADD_TASK) {
    loc = clone_hdl->get_locator(loc_len);
  }
  return (0);
}
