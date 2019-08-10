#include <innodb/clone/Clone_Task_Manager.h>

#include <innodb/clone/Clone_Sys.h>
#include <innodb/memory/mem_heap_alloc.h>
#include <innodb/align/ut_align.h>
#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/clone/print_chunk_info.h>

#include "mysql/plugin.h"  // thd_killed()


#ifdef UNIV_DEBUG
void Clone_Task_Manager::debug_wait(uint chunk_num, Clone_Task *task) {
  auto state = m_clone_snapshot->get_state();
  auto nchunks = m_clone_snapshot->get_num_chunks();

  /* Stop somewhere in the middle of current stage */
  if (!task->m_is_master || task->m_ignore_sync ||
      (chunk_num != 0 && chunk_num < (nchunks / 2 + 1))) {
    return;
  }

  if (state == CLONE_SNAPSHOT_FILE_COPY) {
    DEBUG_SYNC_C("clone_file_copy");

  } else if (state == CLONE_SNAPSHOT_PAGE_COPY) {
    DEBUG_SYNC_C("clone_page_copy");

  } else if (state == CLONE_SNAPSHOT_REDO_COPY) {
    DEBUG_SYNC_C("clone_redo_copy");
  }

  task->m_ignore_sync = true;
}

int Clone_Task_Manager::debug_restart(Clone_Task *task, int in_err,
                                      int restart_count) {
  auto err = in_err;

  if (err != 0 || restart_count < task->m_debug_counter || !task->m_is_master) {
    return (err);
  }

  /* Restart somewhere in the middle of all chunks */
  if (restart_count == 1) {
    auto nchunks = m_clone_snapshot->get_num_chunks();
    auto cur_chunk = task->m_task_meta.m_chunk_num;

    if (cur_chunk != 0 && cur_chunk < (nchunks / 2 + 1)) {
      return (err);
    }
  }

  DBUG_EXECUTE_IF("clone_restart_apply", err = ER_NET_READ_ERROR;);

  if (err != 0) {
    my_error(err, MYF(0));
  }

  /* Allow restart from next point */
  task->m_debug_counter = restart_count + 1;

  return (err);
}
#endif /* UNIV_DEBUG */


void Clone_Task_Manager::init(Clone_Snapshot *snapshot) {
  uint idx;

  m_clone_snapshot = snapshot;

  m_current_state = snapshot->get_state();

  /* ACK state is the previous state of current state */
  if (m_current_state == CLONE_SNAPSHOT_INIT) {
    m_ack_state = CLONE_SNAPSHOT_NONE;
  } else {
    /* If clone is attaching to active snapshot with
    other concurrent clone */
    ut_ad(m_current_state == CLONE_SNAPSHOT_FILE_COPY);
    m_ack_state = CLONE_SNAPSHOT_INIT;
  }

  m_chunk_info.m_total_chunks = 0;

  m_chunk_info.m_min_unres_chunk = 1;
  m_chunk_info.m_max_res_chunk = 0;

  /* Initialize all tasks in inactive state. */
  for (idx = 0; idx < CLONE_MAX_TASKS; idx++) {
    Clone_Task *task;

    task = m_clone_tasks + idx;
    task->m_task_state = CLONE_TASK_INACTIVE;

    task->m_serial_desc = nullptr;
    task->m_alloc_len = 0;

    task->m_current_file_des.m_file = OS_FILE_CLOSED;
    task->m_current_file_index = 0;
    task->m_file_cache = true;

    task->m_current_buffer = nullptr;
    task->m_buffer_alloc_len = 0;
    task->m_is_master = false;
    task->m_has_thd = false;
    task->m_data_size = 0;
    ut_d(task->m_ignore_sync = false);
    ut_d(task->m_debug_counter = 2);
  }

  m_num_tasks = 0;
  m_num_tasks_finished = 0;
  m_num_tasks_transit = 0;
  m_restart_count = 0;

  m_next_state = CLONE_SNAPSHOT_NONE;
  m_send_state_meta = false;
  m_transferred_file_meta = false;
  m_saved_error = 0;

  /* Allocate error file name */
  auto heap = m_clone_snapshot->lock_heap();

  m_err_file_name = static_cast<char *>(mem_heap_alloc(heap, FN_REFLEN_SE));

  m_err_file_len = FN_REFLEN_SE;

  m_clone_snapshot->release_heap(heap);

  /* Initialize error file name */
  memset(m_err_file_name, 0, m_err_file_len);

  strncpy(m_err_file_name, "Clone File", m_err_file_len);
}

void Clone_Task_Manager::reserve_task(THD *thd, uint &task_id) {
  ut_ad(mutex_own(&m_state_mutex));

  Clone_Task *task = nullptr;

  task_id = 0;

  /* Find inactive task in the array. */
  for (; task_id < CLONE_MAX_TASKS; task_id++) {
    task = m_clone_tasks + task_id;
    auto task_meta = &task->m_task_meta;

    if (task->m_task_state == CLONE_TASK_INACTIVE) {
      task->m_task_state = CLONE_TASK_ACTIVE;

      task_meta->m_task_index = task_id;
      task_meta->m_chunk_num = 0;
      task_meta->m_block_num = 0;

      /* Set first task as master task */
      if (task_id == 0) {
        ut_ad(thd != nullptr);
        task->m_is_master = true;
      }

      /* Whether the task has an associated user session */
      task->m_has_thd = (thd != nullptr);

      break;
    }

    task = nullptr;
  }

  ut_ad(task != nullptr);
}

int Clone_Task_Manager::alloc_buffer(Clone_Task *task) {
  if (task->m_alloc_len != 0) {
    /* Task buffers are already allocated in case
    clone operation is restarted. */

    ut_ad(task->m_buffer_alloc_len != 0);
    ut_ad(task->m_serial_desc != nullptr);
    ut_ad(task->m_current_buffer != nullptr);

    return (0);
  }

  /* Allocate task descriptor. */
  auto heap = m_clone_snapshot->lock_heap();

  /* Maximum variable length of descriptor. */
  auto alloc_len =
      static_cast<uint>(m_clone_snapshot->get_max_file_name_length());

  /* Check with maximum path name length. */
  if (alloc_len < FN_REFLEN_SE) {
    alloc_len = FN_REFLEN_SE;
  }

  /* Maximum fixed length of descriptor */
  alloc_len += CLONE_DESC_MAX_BASE_LEN;

  /* Add some buffer. */
  alloc_len += CLONE_DESC_MAX_BASE_LEN;

  ut_ad(task->m_alloc_len == 0);
  ut_ad(task->m_buffer_alloc_len == 0);

  task->m_alloc_len = alloc_len;
  task->m_buffer_alloc_len = m_clone_snapshot->get_dyn_buffer_length();

  alloc_len += task->m_buffer_alloc_len;

  alloc_len += CLONE_ALIGN_DIRECT_IO;

  ut_ad(task->m_serial_desc == nullptr);

  task->m_serial_desc = static_cast<byte *>(mem_heap_alloc(heap, alloc_len));

  m_clone_snapshot->release_heap(heap);

  if (task->m_serial_desc == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(0), alloc_len);
    return (ER_OUTOFMEMORY);
  }

  if (task->m_buffer_alloc_len > 0) {
    task->m_current_buffer = static_cast<byte *>(ut_align(
        task->m_serial_desc + task->m_alloc_len, CLONE_ALIGN_DIRECT_IO));
  }

  return (0);
}

int Clone_Task_Manager::handle_error_other_task(bool set_error) {
  char errbuf[MYSYS_STRERROR_SIZE];

  if (set_error && m_saved_error != 0) {
    ib::info(ER_IB_MSG_151)
        << "Clone error from other task code: " << m_saved_error;
  }

  if (!set_error) {
    return (m_saved_error);
  }

  /* Handle shutdown and KILL */
  if (thd_killed(nullptr)) {
    my_error(ER_QUERY_INTERRUPTED, MYF(0));
    return (ER_QUERY_INTERRUPTED);
  }

  /* Check if DDL has marked for abort. Ignore for client apply. */
  if ((m_clone_snapshot == nullptr || m_clone_snapshot->is_copy()) &&
      Clone_Sys::s_clone_sys_state == CLONE_SYS_ABORT) {
    my_error(ER_DDL_IN_PROGRESS, MYF(0));
    return (ER_DDL_IN_PROGRESS);
  }

  switch (m_saved_error) {
    case ER_DDL_IN_PROGRESS:
    case ER_QUERY_INTERRUPTED:
      my_error(m_saved_error, MYF(0));
      break;

    /* Network errors */
    case ER_NET_PACKET_TOO_LARGE:
    case ER_NET_PACKETS_OUT_OF_ORDER:
    case ER_NET_UNCOMPRESS_ERROR:
    case ER_NET_READ_ERROR:
    case ER_NET_READ_INTERRUPTED:
    case ER_NET_ERROR_ON_WRITE:
    case ER_NET_WRITE_INTERRUPTED:
      my_error(m_saved_error, MYF(0));
      break;

    /* IO Errors */
    case ER_CANT_OPEN_FILE:
    case ER_CANT_CREATE_FILE:
    case ER_ERROR_ON_READ:
    case ER_ERROR_ON_WRITE:
      ut_ad(m_err_file_name != nullptr);
      ut_ad(m_err_file_len != 0);
      my_error(m_saved_error, MYF(0), m_err_file_name, errno,
               my_strerror(errbuf, sizeof(errbuf), errno));
      break;

    case ER_FILE_EXISTS_ERROR:
      ut_ad(m_err_file_name != nullptr);
      ut_ad(m_err_file_len != 0);
      my_error(m_saved_error, MYF(0), m_err_file_name);
      break;

    case ER_WRONG_VALUE:
      ut_ad(m_err_file_name != nullptr);
      ut_ad(m_err_file_len != 0);
      my_error(m_saved_error, MYF(0), "file path", m_err_file_name);
      break;

    case ER_CLONE_REMOTE_ERROR:
      /* Will get the error message from remote */
      break;

    case 0:
      break;

    default:
      my_error(ER_INTERNAL_ERROR, MYF(0),
               "Innodb Clone error in concurrent task");
  }

  return (m_saved_error);
}

bool Clone_Task_Manager::wait_before_add(const byte *ref_loc, uint loc_len) {
  ut_ad(mutex_own(&m_state_mutex));

  /* 1. Don't wait if master task. */
  if (m_num_tasks == 0) {
    return (false);
  }

  /* 2. Wait for state transition to get over */
  if (in_transit_state()) {
    return (true);
  }

  /* 3. For copy state(donor), wait for the state to reach file copy. */
  ut_ad(m_current_state != CLONE_SNAPSHOT_NONE);
  if (ref_loc == nullptr) {
    return (m_current_state == CLONE_SNAPSHOT_INIT);
  }

  Clone_Desc_Locator ref_desc;
  ref_desc.deserialize(ref_loc, loc_len, nullptr);

  ut_ad(m_current_state <= ref_desc.m_state);

  /* 4. For apply state (recipient), wait for apply state to reach
  the copy state in reference locator. */
  if (m_current_state != ref_desc.m_state) {
    return (true);
  }

  /* 4A. For file copy state, wait for all metadata to be transferred. */
  if (m_current_state == CLONE_SNAPSHOT_FILE_COPY &&
      !is_file_metadata_transferred()) {
    return (true);
  }
  return (false);
}

int Clone_Task_Manager::add_task(THD *thd, const byte *ref_loc, uint loc_len,
                                 uint &task_id) {
  mutex_enter(&m_state_mutex);

  /* Check for error from other tasks */
  bool raise_error = (thd != nullptr);

  auto err = handle_error_other_task(raise_error);

  if (err != 0) {
    mutex_exit(&m_state_mutex);
    return (err);
  }

  if (m_num_tasks == CLONE_MAX_TASKS) {
    err = ER_TOO_MANY_CONCURRENT_CLONES;
    my_error(err, MYF(0), CLONE_MAX_TASKS);

    mutex_exit(&m_state_mutex);
    return (err);
  }

  if (wait_before_add(ref_loc, loc_len)) {
    bool is_timeout = false;
    int alert_count = 0;
    err = Clone_Sys::wait_default(
        [&](bool alert, bool &result) {

          ut_ad(mutex_own(&m_state_mutex));
          result = wait_before_add(ref_loc, loc_len);

          /* Check for error from other tasks */
          err = handle_error_other_task(raise_error);

          if (err == 0 && result && alert) {
            /* Print messages every 1 minute - default is 5 seconds. */
            if (++alert_count == 12) {
              alert_count = 0;
              ib::info(ER_IB_MSG_151) << "Clone Add task waiting "
                                         "for state change";
            }
          }
          return (err);
        },
        &m_state_mutex, is_timeout);

    if (err != 0) {
      mutex_exit(&m_state_mutex);
      return (err);

    } else if (is_timeout) {
      ut_ad(false);
      mutex_exit(&m_state_mutex);

      ib::info(ER_IB_MSG_151) << "Clone Add task timed out";

      my_error(ER_INTERNAL_ERROR, MYF(0),
               "Clone Add task failed: "
               "Wait too long for state transition");
      return (ER_INTERNAL_ERROR);
    }
  }

  /* We wait for state transition before adding new task. */
  ut_ad(!in_transit_state());

  reserve_task(thd, task_id);
  ut_ad(task_id <= m_num_tasks);

  ++m_num_tasks;

  mutex_exit(&m_state_mutex);
  return (0);
}

bool Clone_Task_Manager::drop_task(THD *thd, uint task_id, bool &is_master) {
  mutex_enter(&m_state_mutex);

  if (in_transit_state()) {
    ut_ad(m_num_tasks_transit > 0);
    --m_num_tasks_transit;
  }

  ut_ad(m_num_tasks > 0);
  --m_num_tasks;

  auto task = get_task_by_index(task_id);

  add_incomplete_chunk(task);

  reset_chunk(task);

  ut_ad(task->m_task_state == CLONE_TASK_ACTIVE);
  task->m_task_state = CLONE_TASK_INACTIVE;

  is_master = task->m_is_master;

  if (!is_master) {
    mutex_exit(&m_state_mutex);
    return (false);
  }

  /* Master needs to wait for other tasks to get dropped */
  if (m_num_tasks > 0) {
    bool is_timeout = false;
    int alert_count = 0;
    auto err = Clone_Sys::wait_default(
        [&](bool alert, bool &result) {

          ut_ad(mutex_own(&m_state_mutex));
          result = (m_num_tasks > 0);

          if (thd_killed(thd)) {
            return (ER_QUERY_INTERRUPTED);

          } else if (Clone_Sys::s_clone_sys_state == CLONE_SYS_ABORT) {
            return (ER_DDL_IN_PROGRESS);
          }
          if (alert && result) {
            /* Print messages every 1 minute - default is 5 seconds. */
            if (++alert_count == 12) {
              alert_count = 0;
              ib::info(ER_IB_MSG_154) << "Clone Master drop task waiting "
                                         "for other tasks";
            }
          }
          return (0);
        },
        &m_state_mutex, is_timeout);

    if (err != 0) {
      mutex_exit(&m_state_mutex);
      return (false);

    } else if (is_timeout) {
      ut_ad(false);
      ib::info(ER_IB_MSG_154) << "Clone Master drop task timed out";

      mutex_exit(&m_state_mutex);
      return (false);
    }
  }

  mutex_exit(&m_state_mutex);

  /* Restart after network error */
  auto current_err = handle_error_other_task(false);
  if (is_network_error(current_err)) {
    return (true);
  }
  return (false);
}

uint32_t Clone_Task_Manager::get_next_chunk() {
  auto &max_chunk = m_chunk_info.m_max_res_chunk;
  auto &min_chunk = m_chunk_info.m_min_unres_chunk;

  ut_ad(max_chunk <= m_chunk_info.m_total_chunks);

  if (min_chunk > m_chunk_info.m_total_chunks) {
    /* No more chunks left for current state. */
    return (0);
  }

  /* Return the minimum unreserved chunk */
  auto ret_chunk = min_chunk;

  /* Mark the chunk reserved. The chunk must be unreserved. */
  ut_ad(!m_chunk_info.m_reserved_chunks[min_chunk]);
  m_chunk_info.m_reserved_chunks[min_chunk] = true;

  /* Increase max reserved chunk if needed */
  if (max_chunk < min_chunk) {
    max_chunk = min_chunk;
  }

  ut_ad(max_chunk == m_chunk_info.m_reserved_chunks.get_max_set_bit());

  /* Set the next unreserved chunk */
  while (m_chunk_info.m_reserved_chunks[min_chunk]) {
    ++min_chunk;

    /* Exit if all chunks are over */
    if (min_chunk > max_chunk || min_chunk > m_chunk_info.m_total_chunks) {
      ut_ad(min_chunk > m_chunk_info.m_total_chunks ||
            !m_chunk_info.m_reserved_chunks[min_chunk]);

      break;
    }
  }

  return (ret_chunk);
}

uint32_t Clone_Task_Manager::get_next_incomplete_chunk(uint32 &block_num) {
  block_num = 0;

  auto &chunks = m_chunk_info.m_incomplete_chunks;

  if (chunks.empty()) {
    return (0);
  }

  auto it = chunks.begin();

  auto chunk_num = it->first;

  block_num = it->second;

  chunks.erase(it);

  return (chunk_num);
}

int Clone_Task_Manager::reserve_next_chunk(Clone_Task *task,
                                           uint32_t &ret_chunk,
                                           uint32_t &ret_block) {
  mutex_enter(&m_state_mutex);
  ret_chunk = 0;

  /* Check for error from other tasks */
  auto err = handle_error_other_task(task->m_has_thd);
  if (err != 0) {
    mutex_exit(&m_state_mutex);
    return (err);
  }

  if (process_inclomplete_chunk()) {
    /* Get next incomplete chunk. */
    ret_chunk = get_next_incomplete_chunk(ret_block);
    ut_ad(ret_chunk != 0);

  } else {
    /* Get next unreserved chunk. */
    ret_block = 0;
    ret_chunk = get_next_chunk();
  }

  reset_chunk(task);
  mutex_exit(&m_state_mutex);
  return (0);
}

int Clone_Task_Manager::set_chunk(Clone_Task *task, Clone_Task_Meta *new_meta) {
  auto cur_meta = &task->m_task_meta;
  int err = 0;

  ut_ad(cur_meta->m_task_index == new_meta->m_task_index);
  cur_meta->m_task_index = new_meta->m_task_index;

  /* Check if this is a new chunk */
  if (cur_meta->m_chunk_num != new_meta->m_chunk_num) {
    mutex_enter(&m_state_mutex);

    /* Mark the current chunk reserved */
    m_chunk_info.m_reserved_chunks[new_meta->m_chunk_num] = true;

    /* Check and remove the chunk from incomplete chunk list. */
    auto &chunks = m_chunk_info.m_incomplete_chunks;

    auto key_value = chunks.find(new_meta->m_chunk_num);

    if (key_value != chunks.end()) {
      ut_ad(key_value->second < new_meta->m_block_num);
      chunks.erase(key_value);
    }

    reset_chunk(task);

    /* Check for error from other tasks */
    err = handle_error_other_task(task->m_has_thd);

    mutex_exit(&m_state_mutex);

    cur_meta->m_chunk_num = new_meta->m_chunk_num;

#ifdef UNIV_DEBUG
    /* Network failure in the middle of a state */
    err = debug_restart(task, err, 1);

    /* Wait in the middle of state */
    debug_wait(cur_meta->m_chunk_num, task);
#endif /* UNIV_DEBUG */
  }

  cur_meta->m_block_num = new_meta->m_block_num;

  return (err);
}

void Clone_Task_Manager::add_incomplete_chunk(Clone_Task *task) {
  /* Track incomplete chunks during apply */
  if (m_clone_snapshot->is_copy()) {
    return;
  }

  auto &task_meta = task->m_task_meta;

  /* The task doesn't have any incomplete chunks */
  if (task_meta.m_chunk_num == 0) {
    return;
  }

  auto &chunks = m_chunk_info.m_incomplete_chunks;

  chunks[task_meta.m_chunk_num] = task_meta.m_block_num;

  ib::info(ER_IB_MSG_151) << "Clone Apply add inclomple Chunk = "
                          << task_meta.m_chunk_num
                          << " Block = " << task_meta.m_block_num
                          << " Task = " << task_meta.m_task_index;
}

void Clone_Task_Manager::reinit_apply_state(const byte *ref_loc, uint ref_len,
                                            byte *&new_loc, uint &new_len,
                                            uint &alloc_len) {
  ut_ad(m_current_state != CLONE_SNAPSHOT_NONE);
  ut_ad(!m_clone_snapshot->is_copy());

  /* Only master task should be present */
  ut_ad(m_num_tasks == 1);

  /* Reset State transition information */
  reset_transition();

  /* Reset Error information */
  reset_error();

  /* Check if current state is finished and acknowledged */
  ut_ad(m_ack_state <= m_current_state);

  if (m_ack_state == m_current_state) {
    ++m_num_tasks_finished;
  }

  ++m_restart_count;

  switch (m_current_state) {
    case CLONE_SNAPSHOT_INIT:
      ib::info(ER_IB_MSG_151) << "Clone Apply Restarting State: INIT";
      break;

    case CLONE_SNAPSHOT_FILE_COPY:
      ib::info(ER_IB_MSG_151) << "Clone Apply Restarting State: FILE COPY";
      break;

    case CLONE_SNAPSHOT_PAGE_COPY:
      ib::info(ER_IB_MSG_152) << "Clone Apply Restarting State: PAGE COPY";
      break;

    case CLONE_SNAPSHOT_REDO_COPY:
      ib::info(ER_IB_MSG_153) << "Clone Apply Restarting State: REDO COPY";
      break;

    case CLONE_SNAPSHOT_DONE:
      ib::info(ER_IB_MSG_151) << "Clone Apply Restarting State: DONE";
      break;

    case CLONE_SNAPSHOT_NONE:
      /* fall through */

    default:
      ut_ad(false);
  }

  if (m_current_state == CLONE_SNAPSHOT_INIT ||
      m_current_state == CLONE_SNAPSHOT_DONE ||
      m_current_state == CLONE_SNAPSHOT_NONE) {
    new_loc = nullptr;
    new_len = 0;
    return;
  }

  /* Add incomplete chunks from master task */
  auto task = get_task_by_index(0);

  add_incomplete_chunk(task);

  /* Reset task information */
  mutex_enter(&m_state_mutex);
  reset_chunk(task);
  mutex_exit(&m_state_mutex);

  /* Allocate for locator if required */
  Clone_Desc_Locator temp_locator;

  temp_locator.deserialize(ref_loc, ref_len, nullptr);

  /* Update current state information */
  temp_locator.m_state = m_current_state;

  /* Update sub-state information */
  temp_locator.m_metadata_transferred = m_transferred_file_meta;

  auto len = temp_locator.m_header.m_length;
  len += static_cast<uint>(m_chunk_info.get_serialized_length(0));

  if (len > alloc_len) {
    /* Allocate for more for possible reuse */
    len = CLONE_DESC_MAX_BASE_LEN;
    ut_ad(len >= temp_locator.m_header.m_length);

    len += static_cast<uint>(m_chunk_info.get_serialized_length(64));

    auto heap = m_clone_snapshot->lock_heap();

    new_loc = static_cast<byte *>(mem_heap_zalloc(heap, len));
    alloc_len = len;

    m_clone_snapshot->release_heap(heap);
  }

  new_len = alloc_len;

  temp_locator.serialize(new_loc, new_len, &m_chunk_info, nullptr);

  print_chunk_info(&m_chunk_info);
}

void Clone_Task_Manager::reinit_copy_state(const byte *loc, uint loc_len) {
  ut_ad(m_clone_snapshot->is_copy());
  ut_ad(m_num_tasks == 0);

  mutex_enter(&m_state_mutex);

  /* Reset State transition information */
  reset_transition();

  /* Reset Error information */
  reset_error();

  ++m_restart_count;

  switch (m_current_state) {
    case CLONE_SNAPSHOT_INIT:
      ib::info(ER_IB_MSG_151) << "Clone Restarting State: INIT";
      break;

    case CLONE_SNAPSHOT_FILE_COPY:
      ib::info(ER_IB_MSG_151) << "Clone Restarting State: FILE COPY";
      break;

    case CLONE_SNAPSHOT_PAGE_COPY:
      ib::info(ER_IB_MSG_152) << "Clone Restarting State: PAGE COPY";
      break;

    case CLONE_SNAPSHOT_REDO_COPY:
      ib::info(ER_IB_MSG_153) << "Clone Restarting State: REDO COPY";
      break;

    case CLONE_SNAPSHOT_DONE:
      ib::info(ER_IB_MSG_151) << "Clone Restarting State: DONE";
      break;

    case CLONE_SNAPSHOT_NONE:
      /* fall through */

    default:
      ut_ad(false);
  }

  if (m_current_state == CLONE_SNAPSHOT_NONE) {
    ut_ad(false);
    mutex_exit(&m_state_mutex);
    return;
  }

  /* Reset to beginning of current state */
  init_state();

  /* Compare local and remote state */
  Clone_Desc_Locator temp_locator;

  temp_locator.deserialize(loc, loc_len, nullptr);

  /* If Local state is ahead, we must have finished the
  previous state confirmed by ACK. It is enough to
  start from current state. */
  if (temp_locator.m_state != m_current_state) {
#ifdef UNIV_DEBUG
    /* Current state could be just one state ahead */
    if (temp_locator.m_state == CLONE_SNAPSHOT_INIT) {
      ut_ad(m_current_state == CLONE_SNAPSHOT_FILE_COPY);

    } else if (temp_locator.m_state == CLONE_SNAPSHOT_FILE_COPY) {
      ut_ad(m_current_state == CLONE_SNAPSHOT_PAGE_COPY);

    } else if (temp_locator.m_state == CLONE_SNAPSHOT_PAGE_COPY) {
      ut_ad(m_current_state == CLONE_SNAPSHOT_REDO_COPY);

    } else if (temp_locator.m_state == CLONE_SNAPSHOT_REDO_COPY) {
      ut_ad(m_current_state == CLONE_SNAPSHOT_DONE);

    } else {
      ut_ad(false);
    }
#endif /* UNIV_DEBUG */

    /* Apply state is behind. Need to send state metadata */
    m_send_state_meta = true;

    mutex_exit(&m_state_mutex);
    return;
  }

  m_send_state_meta = false;
  m_transferred_file_meta = temp_locator.m_metadata_transferred;

  /* Set progress information for current state */
  temp_locator.deserialize(loc, loc_len, &m_chunk_info);

  m_chunk_info.init_chunk_nums();

  mutex_exit(&m_state_mutex);

  print_chunk_info(&m_chunk_info);
}

void Clone_Task_Manager::init_state() {
  ut_ad(mutex_own(&m_state_mutex));

  auto num_chunks = m_clone_snapshot->get_num_chunks();

  auto heap = m_clone_snapshot->lock_heap();

  m_chunk_info.m_reserved_chunks.reset(num_chunks, heap);

  m_clone_snapshot->release_heap(heap);

  m_chunk_info.m_incomplete_chunks.clear();

  m_chunk_info.m_min_unres_chunk = 1;
  ut_ad(m_chunk_info.m_reserved_chunks.get_min_unset_bit() == 1);

  m_chunk_info.m_max_res_chunk = 0;
  ut_ad(m_chunk_info.m_reserved_chunks.get_max_set_bit() == 0);

  m_chunk_info.m_total_chunks = num_chunks;
}

void Clone_Task_Manager::ack_state(const Clone_Desc_State *state_desc) {
  mutex_enter(&m_state_mutex);

  m_ack_state = state_desc->m_state;
  ut_ad(m_current_state == m_ack_state);
  ib::info(ER_IB_MSG_151) << "Clone set state change ACK: " << m_ack_state;

  mutex_exit(&m_state_mutex);
}

int Clone_Task_Manager::wait_ack(Clone_Handle *clone, Clone_Task *task,
                                 Ha_clone_cbk *callback) {
  mutex_enter(&m_state_mutex);

  ++m_num_tasks_finished;

  /* All chunks are finished */
  reset_chunk(task);

  if (!task->m_is_master) {
    mutex_exit(&m_state_mutex);
    return (0);
  }

  int err = 0;

  if (m_current_state != m_ack_state) {
    bool is_timeout = false;
    int alert_count = 0;
    err = Clone_Sys::wait_default(
        [&](bool alert, bool &result) {

          ut_ad(mutex_own(&m_state_mutex));
          result = (m_current_state != m_ack_state);

          /* Check for error from other tasks */
          err = handle_error_other_task(task->m_has_thd);

          if (err == 0 && result && alert) {
            /* Print messages every 1 minute - default is 5 seconds. */
            if (++alert_count == 12) {
              alert_count = 0;
              ib::info(ER_IB_MSG_151) << "Clone Master waiting "
                                         "for state change ACK ";
            }
            err = clone->send_keep_alive(task, callback);
          }
          return (err);

        },
        &m_state_mutex, is_timeout);

    /* Wait too long */
    if (err == 0 && is_timeout) {
      ut_ad(false);
      ib::info(ER_IB_MSG_151) << "Clone Master wait for state change ACK"
                                 " timed out";

      my_error(ER_INTERNAL_ERROR, MYF(0),
               "Innodb clone state ack wait too long");

      err = ER_INTERNAL_ERROR;
    }
  }
  mutex_exit(&m_state_mutex);

  if (err == 0) {
    ib::info(ER_IB_MSG_151) << "Clone Master received state change ACK";
  }

  return (err);
}

int Clone_Task_Manager::finish_state(Clone_Task *task) {
  mutex_enter(&m_state_mutex);

  if (task->m_is_master) {
    /* Check if ACK was sent before restart */
    if (m_ack_state != m_current_state) {
      ut_ad(m_ack_state < m_current_state);
      ++m_num_tasks_finished;
    } else {
      ut_ad(m_restart_count > 0);
    }
    m_ack_state = m_current_state;

  } else {
    ++m_num_tasks_finished;
  }

  /* All chunks are finished */
  reset_chunk(task);

  /* Check for error from other tasks */
  auto err = handle_error_other_task(task->m_has_thd);

  if (!task->m_is_master || err != 0) {
    mutex_exit(&m_state_mutex);
    return (err);
  }

  ut_ad(task->m_is_master);

#ifdef UNIV_DEBUG
  /* Wait before ending state, if needed */
  if (!task->m_ignore_sync) {
    mutex_exit(&m_state_mutex);
    debug_wait(0, task);
    mutex_enter(&m_state_mutex);
  }
#endif /* UNIV_DEBUG */

  if (m_num_tasks_finished < m_num_tasks) {
    bool is_timeout = false;
    int alert_count = 0;
    err = Clone_Sys::wait_default(
        [&](bool alert, bool &result) {

          ut_ad(mutex_own(&m_state_mutex));
          result = (m_num_tasks_finished < m_num_tasks);

          /* Check for error from other tasks */
          err = handle_error_other_task(task->m_has_thd);

          if (err == 0 && result && alert) {
            /* Print messages every 1 minute - default is 5 seconds. */
            if (++alert_count == 12) {
              alert_count = 0;
              ib::info(ER_IB_MSG_151) << "Clone Apply Master waiting for "
                                         "workers before sending ACK."
                                      << " Total = " << m_num_tasks
                                      << " Finished = " << m_num_tasks_finished;
            }
          }
          return (err);
        },
        &m_state_mutex, is_timeout);

    if (err == 0 && is_timeout) {
      ut_ad(false);
      ib::info(ER_IB_MSG_151) << "Clone Apply Master wait timed out";

      my_error(ER_INTERNAL_ERROR, MYF(0),
               "Clone Apply Master wait timed out before sending ACK");

      err = ER_INTERNAL_ERROR;
    }
  }

  mutex_exit(&m_state_mutex);
  return (err);
}

int Clone_Task_Manager::change_state(Clone_Task *task,
                                     Clone_Desc_State *state_desc,
                                     Snapshot_State new_state, uint &num_wait) {
  mutex_enter(&m_state_mutex);

  num_wait = 0;

  /* Check for error from other tasks */
  auto err = handle_error_other_task(task->m_has_thd);

  if (err != 0) {
    mutex_exit(&m_state_mutex);
    return (err);
  }

  /* First requesting task needs to initiate the state transition. */
  if (!in_transit_state()) {
    m_num_tasks_transit = m_num_tasks;
    m_next_state = new_state;
  }

  /* Master needs to wait for all other tasks. */
  if (task->m_is_master && m_num_tasks_transit > 1) {
    num_wait = m_num_tasks_transit;

    mutex_exit(&m_state_mutex);
    return (0);
  }

  /* Need to wait for transition to next state */
  if (!task->m_is_master) {
    /* Move the current task over to the next state */
    ut_ad(m_num_tasks_transit > 0);
    --m_num_tasks_transit;

    num_wait = m_num_tasks_transit;
    ut_ad(num_wait > 0);

    mutex_exit(&m_state_mutex);
    return (0);
  }

  /* Last task requesting the state change. All other tasks have
  already moved over to next state and waiting for the transition
  to complete. Now it is safe to do the snapshot state transition. */

  ut_ad(task->m_is_master);
  mutex_exit(&m_state_mutex);

  uint num_pending = 0;

  if (m_clone_snapshot->is_copy()) {
    ib::info(ER_IB_MSG_151)
        << "Clone State Change : Number of tasks = " << m_num_tasks;
  } else {
    ib::info(ER_IB_MSG_151)
        << "Clone Apply State Change : Number of tasks = " << m_num_tasks;
  }

  err = m_clone_snapshot->change_state(state_desc, m_next_state,
                                       task->m_current_buffer,
                                       task->m_buffer_alloc_len, num_pending);
  if (err != 0) {
    return (err);
  }

  /* Need to wait for other concurrent clone attached to current snapshot. */
  if (num_pending > 0) {
    bool is_timeout = false;
    int alert_count = 0;
    err = Clone_Sys::wait_default(
        [&](bool alert, bool &result) {

          num_pending = m_clone_snapshot->check_state(m_next_state, false);
          result = (num_pending > 0);

          /* Check for possible shutdown/kill */
          mutex_enter(&m_state_mutex);
          err = handle_error_other_task(task->m_has_thd);
          mutex_exit(&m_state_mutex);

          if (err == 0 && result && alert) {
            /* Print messages every 1 minute - default is 5 seconds. */
            if (++alert_count == 12) {
              alert_count = 0;
              ib::info(ER_IB_MSG_151)
                  << "Clone: master state change waiting for other clone";
            }
          }
          return (err);
        },
        nullptr, is_timeout);

    if (err != 0) {
      /* Exit from state transition */
      num_pending = m_clone_snapshot->check_state(m_next_state, true);
      return (err);

    } else if (is_timeout) {
      /* Exit from state transition */
      num_pending = m_clone_snapshot->check_state(m_next_state, true);
      if (num_pending != 0) {
        ut_ad(false);
        ib::info(ER_IB_MSG_151) << "Clone: master state change timed out";
        my_error(ER_INTERNAL_ERROR, MYF(0),
                 "Clone: master state change wait for other clones timed out: "
                 "Wait too long for state transition");
        return (ER_INTERNAL_ERROR);
      }
    }
  }

  mutex_enter(&m_state_mutex);

  /* Check for error from other tasks. Must finish the state transition
  even in case of an error. */
  err = handle_error_other_task(task->m_has_thd);

  m_current_state = m_next_state;
  m_next_state = CLONE_SNAPSHOT_NONE;

  --m_num_tasks_transit;
  /* In case of error, the other tasks might have exited. */
  ut_ad(m_num_tasks_transit == 0 || err != 0);
  m_num_tasks_transit = 0;

  /* For restart, m_num_tasks_finished may not be up to date */
  ut_ad(m_num_tasks_finished == m_num_tasks || err != 0);
  m_num_tasks_finished = 0;

  ut_d(task->m_ignore_sync = false);
  ut_d(task->m_debug_counter = 0);

  /* Initialize next state after transition. */
  init_state();

  mutex_exit(&m_state_mutex);

  return (err);
}

int Clone_Task_Manager::check_state(Clone_Task *task, Snapshot_State new_state,
                                    bool exit_on_wait, int in_err,
                                    uint32_t &num_wait) {
  mutex_enter(&m_state_mutex);

  num_wait = 0;

  if (in_err != 0) {
    /* Save error for other tasks */
    if (m_saved_error == 0) {
      m_saved_error = in_err;
    }
    /* Mark transit incomplete */
    if (in_transit_state()) {
      ++m_num_tasks_transit;
    }
    mutex_exit(&m_state_mutex);
    return (in_err);
  }

  /* Check for error from other tasks */
  auto err = handle_error_other_task(task->m_has_thd);

  if (err != 0) {
    mutex_exit(&m_state_mutex);
    return (err);
  }

  /* Check if current transition is still in progress. */
  if (in_transit_state() && new_state == m_next_state) {
    num_wait = m_num_tasks_transit;

    ut_ad(num_wait > 0);

    if (exit_on_wait) {
      /* Mark error for other tasks */
      m_saved_error = ER_INTERNAL_ERROR;
      /* Mark transit incomplete */
      ++m_num_tasks_transit;
    }
  }

  mutex_exit(&m_state_mutex);

  return (0);
}
