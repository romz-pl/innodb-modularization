#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Handle.h>
#include <innodb/clone/Clone_Sys_State.h>

/** Clone System */
class Clone_Sys {
 public:
  /** Construct clone system */
  Clone_Sys();

  /** Destructor: Call during system shutdown */
  ~Clone_Sys();

  /** Create and add a new clone handle to clone system
  @param[in]	loc		locator
  @param[in]	hdl_type	handle type
  @param[out]	clone_hdl	clone handle
  @return error code */
  int add_clone(const byte *loc, Clone_Handle_Type hdl_type,
                Clone_Handle *&clone_hdl);

  /** drop a clone handle from clone system
  @param[in]	clone_handle	Clone handle */
  void drop_clone(Clone_Handle *clone_handle);

  /** Find if a clone is already running for the reference locator
  @param[in]	ref_loc		reference locator
  @param[in]	loc_len		reference locator length
  @param[in]	hdl_type	clone type
  @return clone handle if found, NULL otherwise */
  Clone_Handle *find_clone(const byte *ref_loc, uint loc_len,
                           Clone_Handle_Type hdl_type);

  /** Get the clone handle from locator by index
  @param[in]	loc	locator
  @param[in]	loc_len	locator length in bytes
  @return clone handle */
  Clone_Handle *get_clone_by_index(const byte *loc, uint loc_len);

  /** Get or create a snapshot for clone and attach
  @param[in]	hdl_type	handle type
  @param[in]	clone_type	clone type
  @param[in]	snapshot_id	snapshot identifier
  @param[in]	is_pfs_monitor	true, if needs PFS monitoring
  @param[out]	snapshot	clone snapshot
  @return error code */
  int attach_snapshot(Clone_Handle_Type hdl_type, Ha_clone_type clone_type,
                      ib_uint64_t snapshot_id, bool is_pfs_monitor,
                      Clone_Snapshot *&snapshot);

  /** Detach clone handle from snapshot
  @param[in]	snapshot	snapshot
  @param[in]	hdl_type	handle type */
  void detach_snapshot(Clone_Snapshot *snapshot, Clone_Handle_Type hdl_type);

  /** Mark clone state to abort if no active clone. If force is set,
  abort all active clones and set state to abort.
  @param[in]	force	force active clones to abort
  @return true if global state is set to abort successfully */
  bool mark_abort(bool force);

  /** Mark clone state to active if no other abort request */
  void mark_active();

  /** Get next unique ID
  @return unique ID */
  ib_uint64_t get_next_id();

  /** Get clone sys mutex
  @return clone system mutex */
  ib_mutex_t *get_mutex() { return (&m_clone_sys_mutex); }

  /** Clone System state */
  static Clone_Sys_State s_clone_sys_state;

  /** Number of active abort requests */
  static uint s_clone_abort_count;

  /** Function to check wait condition
  @param[in]	is_alert	print alert message
  @param[out]	result		true, if condition is satisfied
  @return error code */
  using Wait_Cond_Cbk_Func = std::function<int(bool, bool &)>;

  /** Wait till the condition is satisfied or timeout.
  @param[in]	sleep_time	sleep time in milliseconds
  @param[in]	timeout		total time to wait in seconds
  @param[in]	alert_interval	alert interval in seconds
  @param[in]	func		callback function for condition check
  @param[in]	mutex		release during sleep and re-acquire
  @param[out]	is_timeout	true if timeout
  @return error code returned by callback function. */
  static int wait(Clone_Msec sleep_time, Clone_Sec timeout,
                  Clone_Sec alert_interval, Wait_Cond_Cbk_Func &&func,
                  ib_mutex_t *mutex, bool &is_timeout) {
    int err = 0;
    bool wait = true;
    is_timeout = false;

    int loop_count = 0;
    auto alert_count = static_cast<int>(alert_interval / sleep_time);
    auto total_count = static_cast<int>(timeout / sleep_time);

    while (!is_timeout && wait && err == 0) {
      ++loop_count;

      /* Release input mutex */
      if (mutex != nullptr) {
        ut_ad(mutex_own(mutex));
        mutex_exit(mutex);
      }

      std::this_thread::sleep_for(sleep_time);

      /* Acquire input mutex back */
      if (mutex != nullptr) {
        mutex_enter(mutex);
      }

      auto alert = (alert_count > 0) ? (loop_count % alert_count == 0) : true;

      err = func(alert, wait);

      is_timeout = (loop_count > total_count);
    }
    return (err);
  }

  /** Wait till the condition is satisfied or default timeout.
  @param[in]	func		callback function for condition check
  @param[in]	mutex		release during sleep and re-acquire
  @param[out]	is_timeout	true if timeout
  @return error code returned by callback function. */
  static int wait_default(Wait_Cond_Cbk_Func &&func, ib_mutex_t *mutex,
                          bool &is_timeout) {
    return (wait(CLONE_DEF_SLEEP, Clone_Sec(CLONE_DEF_TIMEOUT),
                 CLONE_DEF_ALERT_INTERVAL,
                 std::forward<Wait_Cond_Cbk_Func>(func), mutex, is_timeout));
  }

 private:
  /** Check if any active clone is running.
  @param[in]	print_alert	print alert message
  @return true, if concurrent clone in progress */
  bool check_active_clone(bool print_alert);

  /** Find free index to allocate new clone handle.
  @param[in]	hdl_type	clone handle type
  @param[out]	free_index	free index in array
  @return error code */
  int find_free_index(Clone_Handle_Type hdl_type, uint &free_index);

 private:
  /** Array of clone handles */
  Clone_Handle *m_clone_arr[CLONE_ARR_SIZE];

  /** Number of copy clones */
  uint m_num_clones;

  /** Number of apply clones */
  uint m_num_apply_clones;

  /** Array of clone snapshots */
  Clone_Snapshot *m_snapshot_arr[SNAPSHOT_ARR_SIZE];

  /** Number of copy snapshots */
  uint m_num_snapshots;

  /** Number of apply snapshots */
  uint m_num_apply_snapshots;

  /** Clone system mutex */
  ib_mutex_t m_clone_sys_mutex;

  /** Clone unique ID generator */
  ib_uint64_t m_clone_id_generator;
};
