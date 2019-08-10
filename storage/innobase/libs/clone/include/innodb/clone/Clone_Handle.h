#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Handle_State.h>
#include <innodb/clone/Clone_Handle_Type.h>
#include <innodb/clone/Clone_Task_Manager.h>
#include <innodb/clone/Clone_Desc_Locator.h>


/** Clone Handle for copying or applying data */
class Clone_Handle {
 public:
  /** Construct clone handle
  @param[in]	handle_type	clone handle type
  @param[in]	clone_version	clone version
  @param[in]	clone_index	index in clone array */
  Clone_Handle(Clone_Handle_Type handle_type, uint clone_version,
               uint clone_index);

  /** Destructor: Detach from snapshot */
  ~Clone_Handle();

  /** Initialize clone handle
  @param[in]	ref_loc		reference locator
  @param[in]	ref_len		reference locator length
  @param[in]	type		clone type
  @param[in]	data_dir	data directory for apply
  @return error code */
  int init(const byte *ref_loc, uint ref_len, Ha_clone_type type,
           const char *data_dir);

  /** Attach to the clone handle */
  void attach() { ++m_ref_count; }

  /** Detach from the clone handle
  @return reference count */
  uint detach() {
    ut_a(m_ref_count > 0);
    --m_ref_count;

    return (m_ref_count);
  }

  /** Get locator for the clone handle.
  @param[out]	loc_len	serialized locator length
  @return serialized clone locator */
  byte *get_locator(uint &loc_len);

  /** @return clone data directory */
  const char *get_datadir() const { return (m_clone_dir); }

  /** Build locator descriptor for the clone handle
  @param[out]	loc_desc	locator descriptor */
  void build_descriptor(Clone_Desc_Locator *loc_desc);

  /** Add a task to clone handle
  @param[in]	thd	server THD object
  @param[in]	ref_loc	reference locator from remote
  @param[in]	ref_len	reference locator length
  @param[out]	task_id	task identifier
  @return error code */
  int add_task(THD *thd, const byte *ref_loc, uint ref_len, uint &task_id) {
    return (m_clone_task_manager.add_task(thd, ref_loc, ref_len, task_id));
  }

  /** Drop task from clone handle
  @param[in]	thd		server THD object
  @param[in]	task_id		current task ID
  @param[in]	in_err		input error
  @param[out]	is_master	true, if master task
  @return true if needs to wait for re-start */
  bool drop_task(THD *thd, uint task_id, int in_err, bool &is_master);

  /** Save current error number
  @param[in]	err	error number */
  void save_error(int err) {
    if (err != 0) {
      m_clone_task_manager.set_error(err, nullptr);
    }
  }

  /** Check for error from other tasks and DDL
  @param[in,out]	thd	session THD
  @return error code */
  int check_error(THD *thd) {
    bool has_thd = (thd != nullptr);
    auto err = m_clone_task_manager.handle_error_other_task(has_thd);
    /* Save any error reported */
    save_error(err);
    return (err);
  }

  /** @return true if any task is interrupted */
  bool is_interrupted() {
    auto err = m_clone_task_manager.handle_error_other_task(false);
    return (err == ER_QUERY_INTERRUPTED);
  }

  /** Get clone handle index in clone array
  @return array index */
  uint get_index() { return (m_clone_arr_index); }

  /** Get clone data descriptor version
  @return version */
  uint get_version() { return (m_clone_desc_version); }

  /** Check if it is copy clone
  @return true if copy clone handle */
  bool is_copy_clone() { return (m_clone_handle_type == CLONE_HDL_COPY); }

  /** Check if clone type matches
  @param[in]	other_handle_type	type to match with
  @return true if type matches with clone handle type */
  bool match_hdl_type(Clone_Handle_Type other_handle_type) {
    return (m_clone_handle_type == other_handle_type);
  }

  /** Set current clone state
  @param[in]	state	clone handle state */
  void set_state(Clone_Handle_State state) { m_clone_handle_state = state; }

  /** Check if clone state is active
  @return true if in active state */
  bool is_active() { return (m_clone_handle_state == CLONE_STATE_ACTIVE); }

  /** Check if clone is initialized
  @return true if in initial state */
  bool is_init() { return (m_clone_handle_state == CLONE_STATE_INIT); }

  /** Check if clone is idle waiting for restart
  @return true if clone is in idle state */
  bool is_idle() { return (m_clone_handle_state == CLONE_STATE_IDLE); }

  /** Check if clone is aborted
  @return true if clone is aborted */
  bool is_abort() { return (m_clone_handle_state == CLONE_STATE_ABORT); }

  /** Restart copy after a network failure
  @param[in]	thd	server THD object
  @param[in]	loc	locator wit copy state from remote client
  @param[in]	loc_len	locator length in bytes
  @return error code */
  int restart_copy(THD *thd, const byte *loc, uint loc_len);

  /** Build locator with current state and restart apply
  @param[in]	thd	server THD object
  @param[in,out]	loc	loctor with current state information
  @param[in,out]	loc_len	locator length in bytes
  @return error code */
  int restart_apply(THD *thd, const byte *&loc, uint &loc_len);

  /** Transfer snapshot data via callback
  @param[in]	thd		server THD object
  @param[in]	task_id		current task ID
  @param[in]	callback	user callback interface
  @return error code */
  int copy(THD *thd, uint task_id, Ha_clone_cbk *callback);

  /** Apply snapshot data received via callback
  @param[in]	thd		server THD
  @param[in]	task_id		current task ID
  @param[in]	callback	user callback interface
  @return error code */
  int apply(THD *thd, uint task_id, Ha_clone_cbk *callback);

  /** Send keep alive while during long wait
  @param[in]	task		task that is sending the information
  @param[in]	callback	callback interface
  @return error code */
  int send_keep_alive(Clone_Task *task, Ha_clone_cbk *callback);

 private:
  /** Delete clone in progress file. */
  void delete_clone_file();

  /** Create clone data directory.
  @return error code */
  int create_clone_directory();

  /** Display clone progress
  @param[in]	cur_chunk	current chunk number
  @param[in]	max_chunk	total number of chunks
  @param[in,out]	percent_done	percentage completed
  @param[in,out]	disp_time	last displayed time */
  void display_progress(uint32_t cur_chunk, uint32_t max_chunk,
                        uint32_t &percent_done, ulint &disp_time);

  /** Open file for the task
  @param[in]	task		clone task
  @param[in]	file_meta	file information
  @param[in]	file_type	file type (data, log etc.)
  @param[in]	create_file	create if not present
  @param[in]	set_and_close	set size and close
  @return error code */
  int open_file(Clone_Task *task, Clone_File_Meta *file_meta, ulint file_type,
                bool create_file, bool set_and_close);

  /** Close file for the task
  @param[in]	task	clone task
  @return error code */
  int close_file(Clone_Task *task);

  /** Callback providing the file reference and data length to copy
  @param[in]	cbk	callback interface
  @param[in]	task	clone task
  @param[in]	len	data length
  @param[in]	name	file name where func invoked
  @param[in]	line	line where the func invoked
  @return error code */
  int file_callback(Ha_clone_cbk *cbk, Clone_Task *task, uint len
#ifdef UNIV_PFS_IO
                    ,
                    const char *name, uint line
#endif /* UNIV_PFS_IO */
  );

  /** Move to next state
  @param[in]	task		clone task
  @param[in]	callback	callback interface
  @param[in]	state_desc	descriptor for next state to move to
  @return error code */
  int move_to_next_state(Clone_Task *task, Ha_clone_cbk *callback,
                         Clone_Desc_State *state_desc);

  /** Send current state information via callback
  @param[in]	task		task that is sending the information
  @param[in]	callback	callback interface
  @param[in]	is_start	if it is the start of current state
  @return error code */
  int send_state_metadata(Clone_Task *task, Ha_clone_cbk *callback,
                          bool is_start);

  /** Send current task information via callback
  @param[in]	task		task that is sending the information
  @param[in]	callback	callback interface
  @return error code */
  int send_task_metadata(Clone_Task *task, Ha_clone_cbk *callback);

  /** Send all file information via callback
  @param[in]	task		task that is sending the information
  @param[in]	callback	callback interface
  @return error code */
  int send_all_file_metadata(Clone_Task *task, Ha_clone_cbk *callback);

  /** Send current file information via callback
  @param[in]	task		task that is sending the information
  @param[in]	file_meta	file meta information
  @param[in]	callback	callback interface
  @return error code */
  int send_file_metadata(Clone_Task *task, Clone_File_Meta *file_meta,
                         Ha_clone_cbk *callback);

  /** Send cloned data via callback
  @param[in]	task		task that is sending the information
  @param[in]	file_meta	file information
  @param[in]	offset		file offset
  @param[in]	buffer		data buffer or NULL if send from file
  @param[in]	size		data buffer size
  @param[in]	callback	callback interface
  @return error code */
  int send_data(Clone_Task *task, Clone_File_Meta *file_meta,
                ib_uint64_t offset, byte *buffer, uint size,
                Ha_clone_cbk *callback);

  /** Process a data chunk and send data blocks via callback
  @param[in]	task		task that is sending the information
  @param[in]	chunk_num	chunk number to process
  @param[in]	block_num	start block number
  @param[in]	callback	callback interface
  @return error code */
  int process_chunk(Clone_Task *task, uint32_t chunk_num, uint32_t block_num,
                    Ha_clone_cbk *callback);

  /** Create apply task based on task metadata in callback
  @param[in]	task		current task
  @param[in]	callback	callback interface
  @return error code */
  int apply_task_metadata(Clone_Task *task, Ha_clone_cbk *callback);

  /** Move to next state based on state metadata and set
  state information
  @param[in]	task		current task
  @param[in,out]	callback	callback interface
  @param[in,out]	state_desc	clone state descriptor
  @return error code */
  int ack_state_metadata(Clone_Task *task, Ha_clone_cbk *callback,
                         Clone_Desc_State *state_desc);

  /** Move to next state based on state metadata and set
  state information
  @param[in]	task		current task
  @param[in]	callback	callback interface
  @return error code */
  int apply_state_metadata(Clone_Task *task, Ha_clone_cbk *callback);

  /** Create file metadata based on callback
  @param[in]	task		current task
  @param[in]	callback	callback interface
  @return error code */
  int apply_file_metadata(Clone_Task *task, Ha_clone_cbk *callback);

  /** Apply data received via callback
  @param[in]	task		current task
  @param[in]	callback	callback interface
  @return error code */
  int apply_data(Clone_Task *task, Ha_clone_cbk *callback);

  /** Receive data from callback and apply
  @param[in]	task		task that is receiving the information
  @param[in]	offset		file offset for applying data
  @param[in]	file_size	updated file size
  @param[in]	size		data length in bytes
  @param[in]	callback	callback interface
  @return error code */
  int receive_data(Clone_Task *task, uint64_t offset, uint64_t file_size,
                   uint32_t size, Ha_clone_cbk *callback);

 private:
  /** Clone handle type: Copy, Apply */
  Clone_Handle_Type m_clone_handle_type;

  /** Clone handle state */
  Clone_Handle_State m_clone_handle_state;

  /** Fixed locator for version negotiation. */
  byte m_version_locator[CLONE_DESC_MAX_BASE_LEN];

  /** Serialized locator */
  byte *m_clone_locator;

  /** Locator length in bytes */
  uint m_locator_length;

  /** Serialized Restart locator */
  byte *m_restart_loc;

  /** Restart locator length in bytes */
  uint m_restart_loc_len;

  /** Clone descriptor version in use */
  uint m_clone_desc_version;

  /** Index in global array */
  uint m_clone_arr_index;

  /** Unique clone identifier */
  ib_uint64_t m_clone_id;

  /** Reference count */
  uint m_ref_count;

  /** Allow restart of clone operation after network failure */
  bool m_allow_restart;

  /** Clone data directory */
  const char *m_clone_dir;

  /** Clone task manager */
  Clone_Task_Manager m_clone_task_manager;
};
