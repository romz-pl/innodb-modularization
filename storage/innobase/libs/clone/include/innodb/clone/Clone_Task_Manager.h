#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Task.h>
#include <innodb/clone/Clone_Snapshot.h>
#include <innodb/clone/Chunk_Info.h>
#include <innodb/logger/info.h>

class Clone_Handle;

/** Task manager for manging the tasks for a clone operation */
class Clone_Task_Manager {
 public:
  /** Initialize task manager for clone handle
  @param[in]	snapshot	snapshot */
  void init(Clone_Snapshot *snapshot);

  /** Get task state mutex
  @return state mutex */
  ib_mutex_t *get_mutex() { return (&m_state_mutex); }

  /** Handle any error raised by concurrent tasks.
  @param[in]	raise_error	raise error if true
  @return error code */
  int handle_error_other_task(bool raise_error);

  /** Set error number
  @param[in]	err		error number
  @param[in]	file_name	associated file name if any  */
  void set_error(int err, const char *file_name) {
    mutex_enter(&m_state_mutex);

    ib::info(ER_IB_MSG_151) << "Clone Set Error code: " << err
                            << " Saved Error code: " << m_saved_error;

    /* Override any network error as we should not be waiting for restart
    if other errors have occurred. */
    if (m_saved_error == 0 || is_network_error(m_saved_error)) {
      m_saved_error = err;

      if (file_name != nullptr) {
        ut_ad(m_err_file_name != nullptr);
        ut_ad(m_err_file_len != 0);

        strncpy(m_err_file_name, file_name, m_err_file_len);
      }
    }

    mutex_exit(&m_state_mutex);
  }

  /** Add a task to task manager
  @param[in]	thd	server THD object
  @param[in]	ref_loc	reference locator from remote
  @param[in]	loc_len	locator length in bytes
  @param[out]	task_id	task identifier
  @return error code */
  int add_task(THD *thd, const byte *ref_loc, uint loc_len, uint &task_id);

  /** Drop task from task manager
  @param[in]	thd		server THD object
  @param[in]	task_id		current task ID
  @param[out]	is_master	true, if master task
  @return true if needs to wait for re-start */
  bool drop_task(THD *thd, uint task_id, bool &is_master);

  /** Reset chunk information for task
  @param[in]	task	current task */
  void reset_chunk(Clone_Task *task) {
    ut_ad(mutex_own(&m_state_mutex));

    /* Reset current processing chunk */
    task->m_task_meta.m_chunk_num = 0;
    task->m_task_meta.m_block_num = 0;

    if (task->m_data_size > 0) {
      ut_ad(get_state() != CLONE_SNAPSHOT_NONE);
      ut_ad(get_state() != CLONE_SNAPSHOT_INIT);
      ut_ad(get_state() != CLONE_SNAPSHOT_DONE);

      auto &monitor = m_clone_snapshot->get_clone_monitor();

      monitor.update_work(task->m_data_size);
    }

    task->m_data_size = 0;
  }

  /** Get task by index
  @param[in]	index	task index
  @return task */
  Clone_Task *get_task_by_index(uint index) {
    auto task = (m_clone_tasks + index);
    ut_ad(task->m_task_state == CLONE_TASK_ACTIVE);

    return (task);
  }

  /** Reserve next chunk from task manager. Called by individual tasks.
  @param[in]	task		requesting task
  @param[out]	ret_chunk	reserved chunk number
  @param[out]	ret_block	start block number
                                  '0' if no more chunk.
  @return error code */
  int reserve_next_chunk(Clone_Task *task, uint32_t &ret_chunk,
                         uint32_t &ret_block);

  /** Set current chunk and block information
  @param[in,out]	task		requesting task
  @param[in]	new_meta	updated task metadata
  @return error code */
  int set_chunk(Clone_Task *task, Clone_Task_Meta *new_meta);

  /** Track any incomplete chunks handled by the task
  @param[in,out]	task	current task */
  void add_incomplete_chunk(Clone_Task *task);

  /** Initialize task manager for current state */
  void init_state();

  /** Reinitialize state using locator
  @param[in]	loc	locator from remote client
  @param[in]	loc_len	locator length in bytes */
  void reinit_copy_state(const byte *loc, uint loc_len);

  /** Reinitialize state using locator
  @param[in]	ref_loc		current locator
  @param[in]	ref_len		current locator length
  @param[out]	new_loc		new locator to be sent to remote server
  @param[out]	new_len		length of new locator
  @param[in,out]	alloc_len	allocated length for locator buffer */
  void reinit_apply_state(const byte *ref_loc, uint ref_len, byte *&new_loc,
                          uint &new_len, uint &alloc_len);

  /** Reset state transition information */
  void reset_transition() {
    m_num_tasks_transit = 0;
    m_num_tasks_finished = 0;
    m_next_state = CLONE_SNAPSHOT_NONE;
  }

  /** Reset error information */
  void reset_error() {
    m_saved_error = 0;
    strncpy(m_err_file_name, "Clone File", m_err_file_len);
  }

  /** Get current clone state
  @return clone state */
  Snapshot_State get_state() { return (m_current_state); }

  /** Check if in state transition
  @return true if state transition is in progress */
  bool in_transit_state() { return (m_next_state != CLONE_SNAPSHOT_NONE); }

  /** Get attached snapshot
  @return snapshot */
  Clone_Snapshot *get_snapshot() { return (m_clone_snapshot); }

  /** Move to next snapshot state. Each task must call this after
  no more chunk is left in current state. The state can be changed
  only after all tasks have finished transferring the reserved chunks.
  @param[in]	task		clone task
  @param[in]	state_desc	descriptor for next state
  @param[in]	new_state	next state to move to
  @param[out]	num_wait	unfinished tasks in current state
  @return error code */
  int change_state(Clone_Task *task, Clone_Desc_State *state_desc,
                   Snapshot_State new_state, uint &num_wait);

  /** Check if state transition is over and all tasks moved to next state
  @param[in]	task		requesting task
  @param[in]	new_state	next state to move to
  @param[in]	exit_on_wait	exit from transition if needs to wait
  @param[in]	in_err		input error if already occurred
  @param[out]	num_wait	number of tasks to move to next state
  @return error code */
  int check_state(Clone_Task *task, Snapshot_State new_state, bool exit_on_wait,
                  int in_err, uint32_t &num_wait);

  /** Check if needs to send state metadata once
  @param[in]	task	current task
  @return true if needs to send state metadata */
  bool is_restart_metadata(Clone_Task *task) {
    if (task->m_is_master && m_send_state_meta) {
      m_send_state_meta = false;
      return (true);
    }

    return (false);
  }

  /** @return true if file metadata is transferred */
  bool is_file_metadata_transferred() const {
    return (m_transferred_file_meta);
  }

  /** Set sub-state: all file metadata is transferred */
  void set_file_meta_transferred() { m_transferred_file_meta = true; }

  /** Mark state finished for current task
  @param[in]	task	current task
  @return error code */
  int finish_state(Clone_Task *task);

  /** Set acknowledged state
  @param[in]	state_desc	State descriptor */
  void ack_state(const Clone_Desc_State *state_desc);

  /** Wait for acknowledgement
  @param[in]	clone		parent clone handle
  @param[in]	task		current task
  @param[in]	callback	user callback interface
  @return error code */
  int wait_ack(Clone_Handle *clone, Clone_Task *task, Ha_clone_cbk *callback);

  /** Check if state ACK is needed
  @param[in]	state_desc	State descriptor
  @return true if need to wait for ACK from remote */
  bool check_ack(const Clone_Desc_State *state_desc) {
    bool ret = true;

    mutex_enter(&m_state_mutex);

    /* Check if state is already acknowledged */
    if (m_ack_state == state_desc->m_state) {
      ut_ad(m_restart_count > 0);
      ret = false;
      ++m_num_tasks_finished;
    }

    mutex_exit(&m_state_mutex);

    return (ret);
  }

  /** Check if clone is restarted after failure
  @return true if restarted */
  bool is_restarted() { return (m_restart_count > 0); }

  /** Allocate buffers for current task
  @param[in,out]	task	current task
  @return error code */
  int alloc_buffer(Clone_Task *task);

#ifdef UNIV_DEBUG
  /** Wait during clone operation
  @param[in]	chunk_num	chunk number to process
  @param[in]	task		current task */
  void debug_wait(uint chunk_num, Clone_Task *task);

  /** Force restart clone operation by raising network error
  @param[in]	task		current task
  @param[in]	in_err		any err that has occurred
  @param[in]	restart_count	restart counter
  @return error code */
  int debug_restart(Clone_Task *task, int in_err, int restart_count);
#endif /* UNIV_DEBUG */

 private:
  /** Check if we need to wait before adding current task
  @param[in]	ref_loc	reference locator from remote
  @param[in]	loc_len	reference locator length
  @return true, if needs to wait */
  bool wait_before_add(const byte *ref_loc, uint loc_len);

 private:
  /** Check if network error
  @param[in]	err	error code
  @return true if network error */
  bool is_network_error(int err) {
    if (err == ER_NET_ERROR_ON_WRITE || err == ER_NET_READ_ERROR ||
        err == ER_NET_WRITE_INTERRUPTED || err == ER_NET_READ_INTERRUPTED) {
      return (true);
    }
    return (false);
  }

  /** Reserve free task from task manager and initialize
  @param[in]	thd	server THD object
  @param[out]	task_id	initialized task ID */
  void reserve_task(THD *thd, uint &task_id);

  /** Check if we should process incomplete chunk next. Incomplete
  chunks could be there after a re-start from network failure. We always
  process the chunks in order and need to choose accordingly.
  @return if need to process incomplete chunk next. */
  inline bool process_inclomplete_chunk() {
    /* 1. Check if there is any incomplete chunk. */
    auto &chunks = m_chunk_info.m_incomplete_chunks;
    if (chunks.empty()) {
      return (false);
    }

    /* 2. Check if all complete chunks are processed. */
    auto min_complete_chunk = m_chunk_info.m_min_unres_chunk;
    if (min_complete_chunk > m_chunk_info.m_total_chunks) {
      return (true);
    }

    /* 3. Compare the minimum chunk number for complete and incomplete chunk */
    auto it = chunks.begin();
    auto min_incomplete_chunk = it->first;

    ut_ad(min_complete_chunk != min_incomplete_chunk);
    return (min_incomplete_chunk < min_complete_chunk);
  }

  /** Get next in complete chunk if any
  @param[out]	block_num	first block number in chunk
  @return incomplete chunk number */
  uint32_t get_next_incomplete_chunk(uint32 &block_num);

  /** Get next unreserved chunk
  @return chunk number */
  uint32_t get_next_chunk();

 private:
  /** Mutex synchronizing access by concurrent tasks */
  ib_mutex_t m_state_mutex;

  /** Finished and incomplete chunk information */
  Chunk_Info m_chunk_info;

  /** Clone task array */
  Clone_Task m_clone_tasks[CLONE_MAX_TASKS];

  /** Current number of tasks */
  uint m_num_tasks;

  /** Number of tasks finished current state */
  uint m_num_tasks_finished;

  /** Number of tasks in transit state */
  uint m_num_tasks_transit;

  /** Number of times clone is restarted */
  uint m_restart_count;

  /** Acknowledged state from client */
  Snapshot_State m_ack_state;

  /** Current state for clone */
  Snapshot_State m_current_state;

  /** Next state: used during state transfer */
  Snapshot_State m_next_state;

  /* Sub state: File metadata is transferred */
  bool m_transferred_file_meta;

  /** Send state metadata before starting: Used for restart */
  bool m_send_state_meta;

  /** Save any error raised by a task */
  int m_saved_error;

  /** File name related to the saved error */
  char *m_err_file_name;

  /** File name length */
  size_t m_err_file_len;

  /** Attached snapshot handle */
  Clone_Snapshot *m_clone_snapshot;
};
