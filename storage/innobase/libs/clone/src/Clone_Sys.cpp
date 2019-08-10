#include <innodb/clone/Clone_Sys.h>

#include "mysql/plugin.h"  // thd_killed()
#include <innodb/clone/clone_sys.h>
#include <innodb/clone/choose_desc_version.h>
#include <innodb/logger/warn.h>

/** Clone System state */
Clone_Sys_State Clone_Sys::s_clone_sys_state = {CLONE_SYS_INACTIVE};

/** Number of active abort requests */
uint Clone_Sys::s_clone_abort_count = 0;

Clone_Sys::Clone_Sys()
    : m_clone_arr(),
      m_num_clones(),
      m_num_apply_clones(),
      m_snapshot_arr(),
      m_num_snapshots(),
      m_num_apply_snapshots(),
      m_clone_id_generator() {
  mutex_create(LATCH_ID_CLONE_SYS, &m_clone_sys_mutex);
}

Clone_Sys::~Clone_Sys() {
  mutex_free(&m_clone_sys_mutex);

#ifdef UNIV_DEBUG
  /* Verify that no active clone is present */
  int idx;
  for (idx = 0; idx < CLONE_ARR_SIZE; idx++) {
    ut_ad(m_clone_arr[idx] == nullptr);
  }
  ut_ad(m_num_clones == 0);
  ut_ad(m_num_apply_clones == 0);

  for (idx = 0; idx < SNAPSHOT_ARR_SIZE; idx++) {
    ut_ad(m_snapshot_arr[idx] == nullptr);
  }
  ut_ad(m_num_snapshots == 0);
  ut_ad(m_num_apply_snapshots == 0);

#endif /* UNIV_DEBUG */
}

Clone_Handle *Clone_Sys::find_clone(const byte *ref_loc, uint loc_len,
                                    Clone_Handle_Type hdl_type) {
  int idx;
  bool match_found;

  Clone_Desc_Locator loc_desc;
  Clone_Desc_Locator ref_desc;
  Clone_Handle *clone_hdl;

  ut_ad(mutex_own(&m_clone_sys_mutex));

  if (ref_loc == nullptr) {
    return (nullptr);
  }

  ref_desc.deserialize(ref_loc, loc_len, nullptr);

  match_found = false;
  clone_hdl = nullptr;

  for (idx = 0; idx < CLONE_ARR_SIZE; idx++) {
    clone_hdl = m_clone_arr[idx];

    if (clone_hdl == nullptr || clone_hdl->is_init()) {
      continue;
    }

    if (clone_hdl->match_hdl_type(hdl_type)) {
      clone_hdl->build_descriptor(&loc_desc);

      if (loc_desc.match(&ref_desc)) {
        match_found = true;
        break;
      }
    }
  }

  if (match_found) {
    clone_hdl->attach();
    return (clone_hdl);
  }

  return (nullptr);
}

int Clone_Sys::find_free_index(Clone_Handle_Type hdl_type, uint &free_index) {
  free_index = CLONE_ARR_SIZE;

  uint target_index = CLONE_ARR_SIZE;
  Clone_Handle *target_clone = nullptr;

  for (uint idx = 0; idx < CLONE_ARR_SIZE; idx++) {
    auto clone_hdl = m_clone_arr[idx];

    if (clone_hdl == nullptr) {
      free_index = idx;
      break;
    }

    /* If existing clone has some error, it is on its way to exit. */
    auto err = clone_hdl->check_error(nullptr);
    if (hdl_type == CLONE_HDL_COPY && (clone_hdl->is_idle() || err != 0)) {
      target_clone = clone_hdl;
      target_index = idx;
    }
  }

  if (free_index == CLONE_ARR_SIZE ||
      (hdl_type == CLONE_HDL_COPY && m_num_clones == MAX_CLONES) ||
      (hdl_type == CLONE_HDL_APPLY && m_num_apply_clones == MAX_CLONES)) {
    if (target_clone == nullptr) {
      my_error(ER_TOO_MANY_CONCURRENT_CLONES, MYF(0), MAX_CLONES);
      return (ER_TOO_MANY_CONCURRENT_CLONES);
    }
  } else {
    return (0);
  }

  /* We can abort idle clone and use the index. */
  ut_ad(target_clone != nullptr);
  ut_ad(mutex_own(&m_clone_sys_mutex));
  ut_ad(hdl_type == CLONE_HDL_COPY);

  target_clone->set_state(CLONE_STATE_ABORT);

  free_index = target_index;

  /* Sleep for 100 milliseconds. */
  Clone_Msec sleep_time(100);
  /* Generate alert message every second. */
  Clone_Sec alert_interval(1);
  /* Wait for 5 seconds for idle client to abort. */
  Clone_Sec time_out(5);

  bool is_timeout = false;
  auto err = Clone_Sys::wait(
      sleep_time, time_out, alert_interval,
      [&](bool alert, bool &result) {

        ut_ad(mutex_own(clone_sys->get_mutex()));
        auto current_clone = m_clone_arr[target_index];
        result = (current_clone != nullptr);

        if (thd_killed(nullptr)) {
          ib::info(ER_IB_MSG_151)
              << "Clone Begin Master wait for abort interrupted";
          my_error(ER_QUERY_INTERRUPTED, MYF(0));
          return (ER_QUERY_INTERRUPTED);

        } else if (Clone_Sys::s_clone_sys_state == CLONE_SYS_ABORT) {
          ib::info(ER_IB_MSG_151)
              << "Clone Begin Master wait for abort interrupted by DDL";
          my_error(ER_DDL_IN_PROGRESS, MYF(0));
          return (ER_DDL_IN_PROGRESS);

        } else if (result) {
          ut_ad(current_clone->is_abort());
        }

        if (!result) {
          ib::info(ER_IB_MSG_151) << "Clone Master aborted idle task";

        } else if (alert) {
          ib::info(ER_IB_MSG_151) << "Clone Master waiting for idle task abort";
        }
        return (0);
      },
      clone_sys->get_mutex(), is_timeout);

  if (err == 0 && is_timeout) {
    ib::info(ER_IB_MSG_151) << "Clone Master wait for abort timed out";
    my_error(ER_INTERNAL_ERROR, MYF(0),
             "Innodb Clone Copy failed to abort idle clone [timeout]");
    err = ER_INTERNAL_ERROR;
  }
  return (err);
}

int Clone_Sys::add_clone(const byte *loc, Clone_Handle_Type hdl_type,
                         Clone_Handle *&clone_hdl) {
  ut_ad(mutex_own(&m_clone_sys_mutex));
  ut_ad(m_num_clones <= MAX_CLONES);
  ut_ad(m_num_apply_clones <= MAX_CLONES);

  auto version = choose_desc_version(loc);

  /* Find a free index to allocate new clone. */
  uint free_idx;
  auto err = find_free_index(hdl_type, free_idx);
  if (err != 0) {
    return (err);
  }

  /* Create a new clone. */
  clone_hdl = UT_NEW(Clone_Handle(hdl_type, version, free_idx), mem_key_clone);

  if (clone_hdl == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(0), sizeof(Clone_Handle));
    return (ER_OUTOFMEMORY);
  }

  m_clone_arr[free_idx] = clone_hdl;

  if (hdl_type == CLONE_HDL_COPY) {
    ++m_num_clones;
  } else {
    ut_ad(hdl_type == CLONE_HDL_APPLY);
    ++m_num_apply_clones;
  }

  clone_hdl->attach();

  return (0);
}


void Clone_Sys::drop_clone(Clone_Handle *clone_handle) {
  ut_ad(mutex_own(&m_clone_sys_mutex));

  if (clone_handle->detach() > 0) {
    return;
  }

  auto index = clone_handle->get_index();

  ut_ad(m_clone_arr[index] == clone_handle);

  m_clone_arr[index] = nullptr;

  if (clone_handle->is_copy_clone()) {
    ut_ad(m_num_clones > 0);
    --m_num_clones;

  } else {
    ut_ad(m_num_apply_clones > 0);
    --m_num_apply_clones;
  }

  UT_DELETE(clone_handle);
}

Clone_Handle *Clone_Sys::get_clone_by_index(const byte *loc, uint loc_len) {
  Clone_Desc_Locator loc_desc;
  Clone_Handle *clone_hdl;

  loc_desc.deserialize(loc, loc_len, nullptr);

#ifdef UNIV_DEBUG
  Clone_Desc_Header *header = &loc_desc.m_header;
  ut_ad(header->m_type == CLONE_DESC_LOCATOR);
#endif
  clone_hdl = m_clone_arr[loc_desc.m_clone_index];

  ut_ad(clone_hdl != nullptr);

  return (clone_hdl);
}

int Clone_Sys::attach_snapshot(Clone_Handle_Type hdl_type,
                               Ha_clone_type clone_type,
                               ib_uint64_t snapshot_id, bool is_pfs_monitor,
                               Clone_Snapshot *&snapshot) {
  uint idx;
  uint free_idx = SNAPSHOT_ARR_SIZE;

  ut_ad(mutex_own(&m_clone_sys_mutex));

  /* Try to attach to an existing snapshot. */
  for (idx = 0; idx < SNAPSHOT_ARR_SIZE; idx++) {
    snapshot = m_snapshot_arr[idx];

    if (snapshot != nullptr) {
      if (snapshot->attach(hdl_type, is_pfs_monitor)) {
        return (0);
      }
    } else if (free_idx == SNAPSHOT_ARR_SIZE) {
      free_idx = idx;
    }
  }

  if (free_idx == SNAPSHOT_ARR_SIZE ||
      (hdl_type == CLONE_HDL_COPY && m_num_snapshots == MAX_SNAPSHOTS) ||
      (hdl_type == CLONE_HDL_APPLY && m_num_apply_snapshots == MAX_SNAPSHOTS)) {
    my_error(ER_TOO_MANY_CONCURRENT_CLONES, MYF(0), MAX_SNAPSHOTS);
    return (ER_TOO_MANY_CONCURRENT_CLONES);
  }

  /* Create a new snapshot. */
  snapshot = UT_NEW(Clone_Snapshot(hdl_type, clone_type, free_idx, snapshot_id),
                    mem_key_clone);

  if (snapshot == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(0), sizeof(Clone_Snapshot));
    return (ER_OUTOFMEMORY);
  }

  m_snapshot_arr[free_idx] = snapshot;

  if (hdl_type == CLONE_HDL_COPY) {
    ++m_num_snapshots;
  } else {
    ut_ad(hdl_type == CLONE_HDL_APPLY);
    ++m_num_apply_snapshots;
  }

  snapshot->attach(hdl_type, is_pfs_monitor);

  return (0);
}

void Clone_Sys::detach_snapshot(Clone_Snapshot *snapshot,
                                Clone_Handle_Type hdl_type) {
  uint num_clones;

  ut_ad(mutex_own(&m_clone_sys_mutex));
  num_clones = snapshot->detach();

  if (num_clones != 0) {
    return;
  }

  /* All clones are detached. Drop the snapshot. */
  uint index;

  index = snapshot->get_index();
  ut_ad(m_snapshot_arr[index] == snapshot);

  UT_DELETE(snapshot);

  m_snapshot_arr[index] = nullptr;

  if (hdl_type == CLONE_HDL_COPY) {
    ut_ad(m_num_snapshots > 0);
    --m_num_snapshots;

  } else {
    ut_ad(hdl_type == CLONE_HDL_APPLY);
    ut_ad(m_num_apply_snapshots > 0);
    --m_num_apply_snapshots;
  }
}

bool Clone_Sys::check_active_clone(bool print_alert) {
  ut_ad(mutex_own(&m_clone_sys_mutex));

  bool active_clone = false;
  /* Check for active clone operations. */
  for (int idx = 0; idx < CLONE_ARR_SIZE; idx++) {
    auto clone_hdl = m_clone_arr[idx];

    if (clone_hdl != nullptr && clone_hdl->is_copy_clone()) {
      active_clone = true;
      break;
    }
  }

  if (active_clone && print_alert) {
    ib::info(ER_IB_MSG_149) << "DDL waiting for CLONE to abort";
  }
  return (active_clone);
}

bool Clone_Sys::mark_abort(bool force) {
  ut_ad(mutex_own(&m_clone_sys_mutex));

  /* Check for active clone operations. */
  auto active_clone = check_active_clone(false);

  /* If active clone is running and force is not set then
  return without setting abort state. */
  if (active_clone && !force) {
    return (false);
  }

  ++s_clone_abort_count;

  if (s_clone_sys_state != CLONE_SYS_ABORT) {
    ut_ad(s_clone_abort_count == 1);
    s_clone_sys_state = CLONE_SYS_ABORT;

    DEBUG_SYNC_C("clone_marked_abort");
  }

  if (active_clone) {
    ut_ad(force);

    /* Sleep for 1 second */
    Clone_Msec sleep_time(Clone_Sec(1));
    /* Generate alert message every minute. */
    Clone_Sec alert_time(Clone_Min(1));
    /* Timeout in 15 minutes - safeguard against hang, should not happen */
    Clone_Sec time_out(Clone_Min(15));

    bool is_timeout = false;

    wait(sleep_time, time_out, alert_time,
         [&](bool alert, bool &result) {

           ut_ad(mutex_own(&m_clone_sys_mutex));
           result = check_active_clone(alert);

           return (0);

         },
         &m_clone_sys_mutex, is_timeout);

    if (is_timeout) {
      ut_ad(false);
      ib::warn(ER_IB_MSG_150) << "DDL wait for CLONE abort timed out"
                                 ", Continuing DDL.";
    }
  }
  return (true);
}

void Clone_Sys::mark_active() {
  ut_ad(mutex_own(&m_clone_sys_mutex));

  ut_ad(s_clone_abort_count > 0);
  --s_clone_abort_count;

  if (s_clone_abort_count == 0) {
    s_clone_sys_state = CLONE_SYS_ACTIVE;
  }
}

ib_uint64_t Clone_Sys::get_next_id() {
  ut_ad(mutex_own(&m_clone_sys_mutex));

  return (++m_clone_id_generator);
}
