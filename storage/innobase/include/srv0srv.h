/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All rights reserved.
Copyright (c) 2008, 2009, Google Inc.
Copyright (c) 2009, Percona Inc.

Portions of this file contain modifications contributed and copyrighted by
Google, Inc. Those modifications are gratefully acknowledged and are described
briefly in the InnoDB documentation. The contributions by Google are
incorporated with their permission, and subject to the conditions contained in
the file COPYING.Google.

Portions of this file contain modifications contributed and copyrighted
by Percona Inc.. Those modifications are
gratefully acknowledged and are described briefly in the InnoDB
documentation. The contributions by Percona Inc. are incorporated with
their permission, and subject to the conditions contained in the file
COPYING.Percona.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/srv0srv.h
 The server main program

 Created 10/10/1995 Heikki Tuuri
 *******************************************************/

#ifndef srv0srv_h
#define srv0srv_h

#include <innodb/univ/univ.h>

#include <innodb/clone/srv_undo_log_encrypt.h>
#include <innodb/clone/srv_redo_log_encrypt.h>
#include <innodb/log_arch/srv_log_group_home_dir.h>
#include <innodb/log_arch/srv_log_file_size.h>
#include <innodb/log_write/srv_log_writer_spin_delay.h>
#include <innodb/log_write/srv_log_writer_timeout.h>
#include <innodb/log_write/srv_flush_log_at_timeout.h>
#include <innodb/log_write/srv_log_flusher_timeout.h>
#include <innodb/log_write/srv_log_flusher_spin_delay.h>
#include <innodb/log_write/srv_log_write_notifier_timeout.h>
#include <innodb/log_write/srv_log_write_notifier_spin_delay.h>
#include <innodb/log_redo/srv_stats.h>
#include <innodb/log_redo/srv_log_write_max_size.h>
#include <innodb/log_write/srv_log_closer_timeout.h>
#include <innodb/log_write/srv_log_closer_spin_delay.h>
#include <innodb/log_write/srv_flush_log_at_trx_commit.h>
#include <innodb/log_write/srv_log_spin_cpu_abs_lwm.h>
#include <innodb/log_write/srv_cpu_usage.h>
#include <innodb/log_write/Srv_cpu_usage.h>
#include <innodb/time/ib_time_t.h>
#include <innodb/io/os_file_stat_t.h>
#include <innodb/ioasync/srv_reset_io_thread_op_info.h>
#include <innodb/ioasync/srv_set_io_thread_op_info.h>
#include <innodb/buffer/srv_buf_pool_instances.h>
#include <innodb/buffer/buf_pool_get_curr_size.h>
#include <innodb/buffer/srv_buf_pool_old_size.h>
#include <innodb/buffer/srv_buf_pool_curr_size.h>
#include <innodb/buffer/srv_buf_pool_base_size.h>
#include <innodb/buffer/srv_buf_pool_size.h>
#include <innodb/buffer/srv_buf_pool_min_size.h>
#include <innodb/buffer/srv_buf_pool_def_size.h>
#include <innodb/buffer/srv_buf_pool_chunk_unit.h>
#include <innodb/buffer/srv_buf_pool_instances_default.h>
#include <innodb/buffer/srv_buf_pool_dump_pct.h>
#include <innodb/log_redo/srv_log_write_ahead_size.h>
#include <innodb/log_flush/srv_log_flush_events.h>
#include <innodb/log_buffer/srv_log_buffer_size.h>
#include <innodb/log_redo/srv_log_write_events.h>
#include <innodb/log_redo/srv_log_recent_written_size.h>
#include <innodb/log_redo/srv_log_recent_closed_size.h>
#include <innodb/log_write/srv_log_wait_for_flush_spin_delay.h>
#include <innodb/log_write/srv_log_wait_for_flush_spin_hwm.h>
#include <innodb/log_write/srv_log_wait_for_flush_timeout.h>
#include <innodb/log_write/srv_log_flush_notifier_timeout.h>
#include <innodb/log_write/srv_log_flush_notifier_spin_delay.h>


#include "buf0checksum.h"
#include "fil0fil.h"

#include "mysql/psi/mysql_stage.h"

#ifndef UNIV_HOTBACKUP
#include "log0ddl.h"

#include "que0types.h"
#include "srv0conc.h"
#include "trx0types.h"
#include <innodb/counter/counter.h>
#include <innodb/log_redo/srv_stats_t.h>



struct Srv_threads {
  /** true if monitor thread is created */
  bool m_monitor_thread_active;

  /** true if error monitor thread is created */
  bool m_error_monitor_thread_active;

  /** true if buffer pool dump/load thread is created */
  bool m_buf_dump_thread_active;

  /** true if buffer pool resize thread is created */
  bool m_buf_resize_thread_active;

  /** true if stats thread is created */
  bool m_dict_stats_thread_active;

  /** true if timeout thread is created */
  bool m_timeout_thread_active;

  /** true if master thread is created */
  bool m_master_thread_active;

  /** true if tablespace alter encrypt thread is created */
  bool m_ts_alter_encrypt_thread_active;
};



/** Structure with state of srv background threads. */
extern Srv_threads srv_threads;


extern Log_DDL *log_ddl;

#ifdef INNODB_DD_TABLE
extern bool srv_is_upgrade_mode;
extern bool srv_downgrade_logs;
extern bool srv_upgrade_old_undo_found;
#endif /* INNODB_DD_TABLE */

extern const char *srv_main_thread_op_info;

/* The monitor thread waits on this event. */
extern os_event_t srv_monitor_event;

/* The error monitor thread waits on this event. */
extern os_event_t srv_error_event;

/** The buffer pool dump/load thread waits on this event. */
extern os_event_t srv_buf_dump_event;

/** The buffer pool resize thread waits on this event. */
extern os_event_t srv_buf_resize_event;
#endif /* !UNIV_HOTBACKUP */


extern char *srv_buf_dump_filename;

/** Boolean config knobs that tell InnoDB to dump the buffer pool at shutdown
and/or load it during startup. */
extern bool srv_buffer_pool_dump_at_shutdown;
extern bool srv_buffer_pool_load_at_startup;

/* Whether to disable file system cache if it is defined */
extern bool srv_disable_sort_file_cache;

/* If the last data file is auto-extended, we add this many pages to it
at a time */
#define SRV_AUTO_EXTEND_INCREMENT (srv_sys_space.get_autoextend_increment())

#ifndef UNIV_HOTBACKUP
/** Mutex protecting page_zip_stat_per_index */
extern ib_mutex_t page_zip_stat_per_index_mutex;
/* Mutex for locking srv_monitor_file. Not created if srv_read_only_mode */
extern ib_mutex_t srv_monitor_file_mutex;
/* Temporary file for innodb monitor output */
extern FILE *srv_monitor_file;
/* Mutex for locking srv_dict_tmpfile. Only created if !srv_read_only_mode.
This mutex has a very high rank; threads reserving it should not
be holding any InnoDB latches. */
extern ib_mutex_t srv_dict_tmpfile_mutex;
/* Temporary file for output from the data dictionary */
extern FILE *srv_dict_tmpfile;
/* Mutex for locking srv_misc_tmpfile. Only created if !srv_read_only_mode.
This mutex has a very low rank; threads reserving it should not
acquire any further latches or sleep before releasing this one. */
extern ib_mutex_t srv_misc_tmpfile_mutex;
/* Temporary file for miscellanous diagnostic output */
extern FILE *srv_misc_tmpfile;
#endif /* !UNIV_HOTBACKUP */

/* Server parameters which are read from the initfile */

extern char *srv_data_home;


/** Set if InnoDB operates in read-only mode or innodb-force-recovery
is greater than SRV_FORCE_NO_TRX_UNDO. */
extern bool high_level_read_only;
/** store to its own file each table created by an user; data
dictionary tables are in the system tablespace 0 */
extern bool srv_file_per_table;
/** Sleep delay for threads waiting to enter InnoDB. In micro-seconds. */
extern ulong srv_thread_sleep_delay;
/** Maximum sleep delay (in micro-seconds), value of 0 disables it.*/
extern ulong srv_adaptive_max_sleep_delay;

/** Sort buffer size in index creation */
extern ulong srv_sort_buf_size;

/** Maximum modification log file size for online index creation */
extern unsigned long long srv_online_max_size;

/** Number of threads to use for parallel reads. */
extern ulong srv_parallel_read_threads;


extern bool srv_numa_interleave;

/** Server undo tablespaces directory, can be absolute path. */
extern char *srv_undo_dir;

/** Number of undo tablespaces to use. */
extern ulong srv_undo_tablespaces;

/** The number of rollback segments per tablespace */
extern ulong srv_rollback_segments;

/** Maximum size of undo tablespace. */
extern unsigned long long srv_max_undo_tablespace_size;

/** Rate at which UNDO records should be purged. */
extern ulong srv_purge_rseg_truncate_frequency;

/** Enable or Disable Truncate of UNDO tablespace. */
extern bool srv_undo_log_truncate;






/** Maximum number of srv_n_log_files, or innodb_log_files_in_group */
#define SRV_N_LOG_FILES_MAX 100
extern ulong srv_n_log_files;

#ifdef UNIV_DEBUG_DEDICATED
extern ulong srv_debug_system_mem_size;
#endif /* UNIV_DEBUG_DEDICATED */



/** The value of the startup parameter innodb_log_file_size. */
extern ulonglong srv_log_file_size_requested;






















/** Number of milliseconds every which a periodical checkpoint is written
by the log checkpointer thread (unless periodical checkpoints are disabled,
which is a case during initial phase of startup). */
extern ulong srv_log_checkpoint_every;

















/** Whether to generate and require checksums on the redo log pages. */
extern bool srv_log_checksums;

#ifdef UNIV_DEBUG

/** If true then disable checkpointing. */
extern bool srv_checkpoint_disabled;

/* Used to inject a failure to find a free rollback segment. */
extern bool srv_inject_too_many_concurrent_trxs;

#endif /* UNIV_DEBUG */



extern ulong srv_log_write_ahead_size;
extern bool srv_adaptive_flushing;
extern bool srv_flush_sync;

/* If this flag is TRUE, then we will load the indexes' (and tables') metadata
even if they are marked as "corrupted". Mostly it is for DBA to process
corrupted index and table */
extern bool srv_load_corrupted;

/** Dedicated server setting */
extern bool srv_dedicated_server;

/** Number of locks to protect buf_pool->page_hash */
extern ulong srv_n_page_hash_locks;
/** Scan depth for LRU flush batch i.e.: number of blocks scanned*/
extern ulong srv_LRU_scan_depth;
/** Whether or not to flush neighbors of a block */
extern ulong srv_flush_neighbors;
/** Lock table size in bytes */
extern ulint srv_lock_table_size;

extern ulint srv_n_file_io_threads;
extern bool srv_random_read_ahead;
extern ulong srv_read_ahead_threshold;
extern ulong srv_n_read_io_threads;
extern ulong srv_n_write_io_threads;

extern uint srv_change_buffer_max_size;

/* Number of IO operations per second the server can do */
extern ulong srv_io_capacity;

/* We use this dummy default value at startup for max_io_capacity.
The real value is set based on the value of io_capacity. */
#define SRV_MAX_IO_CAPACITY_DUMMY_DEFAULT (~0UL)
#define SRV_MAX_IO_CAPACITY_LIMIT (~0UL)
extern ulong srv_max_io_capacity;
/* Returns the number of IO operations that is X percent of the
capacity. PCT_IO(5) -> returns the number of IO operations that
is 5% of the max where max is srv_io_capacity.  */
#define PCT_IO(p) ((ulong)(srv_io_capacity * ((double)(p) / 100.0)))

/** Maximum number of purge threads, including the purge coordinator */
#define MAX_PURGE_THREADS 32

/* The "innodb_stats_method" setting, decides how InnoDB is going
to treat NULL value when collecting statistics. It is not defined
as enum type because the configure option takes unsigned integer type. */
extern ulong srv_innodb_stats_method;

extern ulint srv_max_n_open_files;

extern ulong srv_n_page_cleaners;

extern double srv_max_dirty_pages_pct;
extern double srv_max_dirty_pages_pct_lwm;

extern ulong srv_adaptive_flushing_lwm;


extern ulong srv_force_recovery;
#ifdef UNIV_DEBUG
extern ulong srv_force_recovery_crash;
#endif /* UNIV_DEBUG */

/** The value of the configuration parameter innodb_fast_shutdown,
controlling the InnoDB shutdown.

If innodb_fast_shutdown=0, InnoDB shutdown will purge all undo log
records (except XA PREPARE transactions) and complete the merge of the
entire change buffer, and then shut down the redo log.

If innodb_fast_shutdown=1, InnoDB shutdown will only flush the buffer
pool to data files, cleanly shutting down the redo log.

If innodb_fast_shutdown=2, shutdown will effectively 'crash' InnoDB
(but lose no committed transactions). */
extern ulong srv_fast_shutdown;
extern ibool srv_innodb_status;

extern unsigned long long srv_stats_transient_sample_pages;
extern bool srv_stats_persistent;
extern unsigned long long srv_stats_persistent_sample_pages;
extern bool srv_stats_auto_recalc;
extern bool srv_stats_include_delete_marked;

extern ibool srv_use_doublewrite_buf;
extern ulong srv_doublewrite_batch_size;
extern ulong srv_checksum_algorithm;

extern double srv_max_buf_pool_modified_pct;
extern ulong srv_max_purge_lag;
extern ulong srv_max_purge_lag_delay;

extern ulong srv_replication_delay;
/*-------------------------------------------*/

extern bool srv_print_innodb_monitor;
extern bool srv_print_innodb_lock_monitor;

extern ulong srv_n_spin_wait_rounds;
extern ulong srv_n_free_tickets_to_enter;
extern ulong srv_thread_sleep_delay;
extern ulong srv_spin_wait_delay;
extern ibool srv_priority_boost;

extern ulint srv_truncated_status_writes;

#if defined UNIV_DEBUG || defined UNIV_IBUF_DEBUG
extern bool srv_ibuf_disable_background_merge;
#endif /* UNIV_DEBUG || UNIV_IBUF_DEBUG */

#ifdef UNIV_DEBUG
extern bool srv_buf_pool_debug;
extern bool srv_sync_debug;
extern bool srv_purge_view_update_only_debug;

/** Value of MySQL global used to disable master thread. */
extern bool srv_master_thread_disabled_debug;
#endif /* UNIV_DEBUG */

extern ulint srv_fatal_semaphore_wait_threshold;
#define SRV_SEMAPHORE_WAIT_EXTENSION 7200
extern ulint srv_dml_needed_delay;

#include <innodb/ioasync/SRV_MAX_N_IO_THREADS.h>
#include <innodb/ioasync/srv_io_thread_op_info.h>
#include <innodb/ioasync/srv_io_thread_function.h>

/* the number of purge threads to use from the worker pool (currently 0 or 1) */
extern ulong srv_n_purge_threads;

/* the number of pages to purge in one batch */
extern ulong srv_purge_batch_size;

/* the number of sync wait arrays */
extern ulong srv_sync_array_size;

/* print all user-level transactions deadlocks to mysqld stderr */
extern bool srv_print_all_deadlocks;

/** Print all DDL logs to mysqld stderr */
extern bool srv_print_ddl_logs;

extern bool srv_cmp_per_index_enabled;

/** Status variables to be passed to MySQL */
extern struct export_var_t export_vars;

#ifndef UNIV_HOTBACKUP


/* Keys to register InnoDB threads with performance schema */

#include <innodb/log_arch/pfs.h>

#ifdef UNIV_PFS_THREAD
extern mysql_pfs_key_t buf_dump_thread_key;
extern mysql_pfs_key_t buf_resize_thread_key;
extern mysql_pfs_key_t dict_stats_thread_key;
extern mysql_pfs_key_t fts_optimize_thread_key;
extern mysql_pfs_key_t fts_parallel_merge_thread_key;
extern mysql_pfs_key_t fts_parallel_tokenization_thread_key;
extern mysql_pfs_key_t io_handler_thread_key;
extern mysql_pfs_key_t io_ibuf_thread_key;
extern mysql_pfs_key_t io_log_thread_key;
extern mysql_pfs_key_t io_read_thread_key;
extern mysql_pfs_key_t io_write_thread_key;
extern mysql_pfs_key_t log_writer_thread_key;
extern mysql_pfs_key_t log_closer_thread_key;
extern mysql_pfs_key_t log_checkpointer_thread_key;
extern mysql_pfs_key_t log_flusher_thread_key;
extern mysql_pfs_key_t log_write_notifier_thread_key;
extern mysql_pfs_key_t log_flush_notifier_thread_key;
extern mysql_pfs_key_t page_flush_coordinator_thread_key;
extern mysql_pfs_key_t page_flush_thread_key;
extern mysql_pfs_key_t recv_writer_thread_key;
extern mysql_pfs_key_t srv_error_monitor_thread_key;
extern mysql_pfs_key_t srv_lock_timeout_thread_key;
extern mysql_pfs_key_t srv_master_thread_key;
extern mysql_pfs_key_t srv_monitor_thread_key;
extern mysql_pfs_key_t srv_purge_thread_key;
extern mysql_pfs_key_t srv_worker_thread_key;
extern mysql_pfs_key_t trx_recovery_rollback_thread_key;
extern mysql_pfs_key_t srv_ts_alter_encrypt_thread_key;
#endif /* UNIV_PFS_THREAD */
#endif /* !UNIV_HOTBACKUP */

#ifdef HAVE_PSI_STAGE_INTERFACE
/** Performance schema stage event for monitoring ALTER TABLE progress
everything after flush log_make_latest_checkpoint(). */
extern PSI_stage_info srv_stage_alter_table_end;

/** Performance schema stage event for monitoring ALTER TABLE progress
log_make_latest_checkpoint(). */
extern PSI_stage_info srv_stage_alter_table_flush;

/** Performance schema stage event for monitoring ALTER TABLE progress
row_merge_insert_index_tuples(). */
extern PSI_stage_info srv_stage_alter_table_insert;

/** Performance schema stage event for monitoring ALTER TABLE progress
row_log_apply(). */
extern PSI_stage_info srv_stage_alter_table_log_index;

/** Performance schema stage event for monitoring ALTER TABLE progress
row_log_table_apply(). */
extern PSI_stage_info srv_stage_alter_table_log_table;

/** Performance schema stage event for monitoring ALTER TABLE progress
row_merge_sort(). */
extern PSI_stage_info srv_stage_alter_table_merge_sort;

/** Performance schema stage event for monitoring ALTER TABLE progress
row_merge_read_clustered_index(). */
extern PSI_stage_info srv_stage_alter_table_read_pk_internal_sort;

/** Performance schema stage event for monitoring ALTER TABLESPACE
ENCRYPTION progress. */
extern PSI_stage_info srv_stage_alter_tablespace_encryption;

/** Performance schema stage event for monitoring buffer pool load progress. */
extern PSI_stage_info srv_stage_buffer_pool_load;
#endif /* HAVE_PSI_STAGE_INTERFACE */



#include <innodb/io/srv_is_direct_io.h>


#ifndef _WIN32

#include <innodb/io/srv_unix_flush_t.h>
#include <innodb/io/srv_unix_file_flush_method.h>





#else
/** Alternatives for file i/o in Windows. @see innodb_flush_method_names. */
enum srv_win_flush_t {
  /** unbuffered I/O; this is the default */
  SRV_WIN_IO_UNBUFFERED = 0,
  /** buffered I/O */
  SRV_WIN_IO_NORMAL,
};
extern enum srv_win_flush_t srv_win_file_flush_method;



#endif /* _WIN32 */

/** Alternatives for srv_force_recovery. Non-zero values are intended
to help the user get a damaged database up so that he can dump intact
tables and rows with SELECT INTO OUTFILE. The database must not otherwise
be used with these options! A bigger number below means that all precautions
of lower numbers are included. */
enum {
  SRV_FORCE_IGNORE_CORRUPT = 1,   /*!< let the server run even if it
                                  detects a corrupt page */
  SRV_FORCE_NO_BACKGROUND = 2,    /*!< prevent the main thread from
                                  running: if a crash would occur
                                  in purge, this prevents it */
  SRV_FORCE_NO_TRX_UNDO = 3,      /*!< do not run trx rollback after
                                  recovery */
  SRV_FORCE_NO_IBUF_MERGE = 4,    /*!< prevent also ibuf operations:
                                  if they would cause a crash, better
                                  not do them */
  SRV_FORCE_NO_UNDO_LOG_SCAN = 5, /*!< do not look at undo logs when
                                  starting the database: InnoDB will
                                  treat even incomplete transactions
                                  as committed */
  SRV_FORCE_NO_LOG_REDO = 6       /*!< do not do the log roll-forward
                                  in connection with recovery */
};

/* Alternatives for srv_innodb_stats_method, which could be changed by
setting innodb_stats_method */
enum srv_stats_method_name_enum {
  SRV_STATS_NULLS_EQUAL,   /* All NULL values are treated as
                           equal. This is the default setting
                           for innodb_stats_method */
  SRV_STATS_NULLS_UNEQUAL, /* All NULL values are treated as
                           NOT equal. */
  SRV_STATS_NULLS_IGNORED  /* NULL values are ignored */
};

typedef enum srv_stats_method_name_enum srv_stats_method_name_t;

#ifdef UNIV_DEBUG
/** Force all user tables to use page compression. */
extern ulong srv_debug_compress;
#endif /* UNIV_DEBUG */

/** Types of threads existing in the system. */
enum srv_thread_type {
  SRV_NONE,   /*!< None */
  SRV_WORKER, /*!< threads serving parallelized
              queries and queries released from
              lock wait */
  SRV_PURGE,  /*!< Purge coordinator thread */
  SRV_MASTER  /*!< the master thread, (whose type
              number must be biggest) */
};

/** Boots Innobase server. */
void srv_boot(void);
/** Frees the data structures created in srv_init(). */
void srv_free(void);

/** Tells the purge thread that there has been activity in the database
 and wakes up the purge thread if it is suspended (not sleeping).  Note
 that there is a small chance that the purge thread stays suspended
 (we do not protect our operation with the srv_sys_t:mutex, for
 performance reasons). */
void srv_wake_purge_thread_if_not_active(void);
/** Tells the Innobase server that there has been activity in the database
 and wakes up the master thread if it is suspended (not sleeping). Used
 in the MySQL interface. Note that there is a small chance that the master
 thread stays suspended (we do not protect our operation with the kernel
 mutex, for performace reasons). */
void srv_active_wake_master_thread_low(void);
#define srv_active_wake_master_thread()    \
  do {                                     \
    if (!srv_read_only_mode) {             \
      srv_active_wake_master_thread_low(); \
    }                                      \
  } while (0)
/** Wakes up the master thread if it is suspended or being suspended. */
void srv_wake_master_thread(void);
#ifndef UNIV_HOTBACKUP
/** Outputs to a file the output of the InnoDB Monitor.
 @return false if not all information printed
 due to failure to obtain necessary mutex */
ibool srv_printf_innodb_monitor(
    FILE *file,       /*!< in: output stream */
    ibool nowait,     /*!< in: whether to wait for the
                      lock_sys_t::mutex */
    ulint *trx_start, /*!< out: file position of the start of
                      the list of active transactions */
    ulint *trx_end);  /*!< out: file position of the end of
                      the list of active transactions */

/** Function to pass InnoDB status variables to MySQL */
void srv_export_innodb_status(void);
/** Get current server activity count. We don't hold srv_sys::mutex while
 reading this value as it is only used in heuristics.
 @return activity count. */
ulint srv_get_activity_count(void);
/** Check if there has been any activity.
 @return false if no change in activity counter. */
ibool srv_check_activity(ulint old_activity_count); /*!< old activity count */
/** Increment the server activity counter. */
void srv_inc_activity_count(void);

/** Enqueues a task to server task queue and releases a worker thread, if there
 is a suspended one. */
void srv_que_task_enqueue_low(que_thr_t *thr); /*!< in: query thread */

/** A thread which prints the info output by various InnoDB monitors. */
void srv_monitor_thread();

/** A thread which prints warnings about semaphore waits which have lasted
too long. These can be used to track bugs which cause hangs. */
void srv_error_monitor_thread();

/** The master thread controlling the server. */
void srv_master_thread();

/** Purge coordinator thread that schedules the purge tasks. */
void srv_purge_coordinator_thread();

/** Worker thread that reads tasks from the work queue and executes them. */
void srv_worker_thread();

/** Rotate default master key for UNDO tablespace. */
void undo_rotate_default_master_key();

/** Enable UNDO tablespace encryption.
@param[in] is_boot	true if it is called during server start up. In this
                        case, default master key will be used which will be
                        rotated later with actual master key from kyering.
@return false for success, true otherwise. */
bool srv_enable_undo_encryption(bool is_boot);

/** Enable REDO tablespace encryption.
@param[in] is_boot	true if it is called during server start up. In this
                        case, default master key will be used which will be
                        rotated later with actual master key from kyering.
@return false for success, true otherwise. */
bool srv_enable_redo_encryption(bool is_boot);

/** Get count of tasks in the queue.
 @return number of tasks in queue */
ulint srv_get_task_queue_length(void);

/** Releases threads of the type given from suspension in the thread table.
 NOTE! The server mutex has to be reserved by the caller!
 @return number of threads released: this may be less than n if not
 enough threads were suspended at the moment */
ulint srv_release_threads(enum srv_thread_type type, /*!< in: thread type */
                          ulint n); /*!< in: number of threads to release */

/** Check whether any background thread is created.
Send the threads wakeup signal.

NOTE: this check is part of the final shutdown, when the first phase of
shutdown has already been completed.
@see srv_pre_dd_shutdown()
@see srv_master_thread_active()
@return name of thread that is active
@retval NULL if no thread is active */
const char *srv_any_background_threads_are_active();

/** Check whether the master thread is active.
This is polled during the final phase of shutdown.
The first phase of server shutdown must have already been executed
(or the server must not have been fully started up).
@see srv_pre_dd_shutdown()
@see srv_any_background_threads_are_active()
@retval true   if any thread is active
@retval false  if no thread is active */
bool srv_master_thread_active();

/** Wakeup the purge threads. */
void srv_purge_wakeup(void);

/** Check if the purge threads are active, both coordinator and worker threads
@return true if any thread is active, false if no thread is active */
bool srv_purge_threads_active();

/** Create an undo tablespace with an explicit file name
@param[in]	space_name	tablespace name
@param[in]	file_name	file name
@param[out]	space_id	Tablespace ID chosen
@return DB_SUCCESS or error code */
dberr_t srv_undo_tablespace_create(const char *space_name,
                                   const char *file_name, space_id_t space_id);

/** Initialize undo::spaces and trx_sys_undo_spaces,
called once during srv_start(). */
void undo_spaces_init();

/** Free the resources occupied by undo::spaces and trx_sys_undo_spaces,
called once during thread de-initialization. */
void undo_spaces_deinit();

#ifdef UNIV_DEBUG
struct SYS_VAR;

/** Disables master thread. It's used by:
        SET GLOBAL innodb_master_thread_disabled_debug = 1 (0).
@param[in]	thd		thread handle
@param[in]	var		pointer to system variable
@param[out]	var_ptr		where the formal string goes
@param[in]	save		immediate result from check function */
void srv_master_thread_disabled_debug_update(THD *thd, SYS_VAR *var,
                                             void *var_ptr, const void *save);
#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */

/** Status variables to be passed to MySQL */
struct export_var_t {
  ulint innodb_data_pending_reads;  /*!< Pending reads */
  ulint innodb_data_pending_writes; /*!< Pending writes */
  ulint innodb_data_pending_fsyncs; /*!< Pending fsyncs */
  ulint innodb_data_fsyncs;         /*!< Number of fsyncs so far */
  ulint innodb_data_read;           /*!< Data bytes read */
  ulint innodb_data_writes;         /*!< I/O write requests */
  ulint innodb_data_written;        /*!< Data bytes written */
  ulint innodb_data_reads;          /*!< I/O read requests */
  char innodb_buffer_pool_dump_status[OS_FILE_MAX_PATH +
                                      128]; /*!< Buf pool dump status */
  char innodb_buffer_pool_load_status[OS_FILE_MAX_PATH +
                                      128];   /*!< Buf pool load status */
  char innodb_buffer_pool_resize_status[512]; /*!< Buf pool resize status */
  ulint innodb_buffer_pool_pages_total;       /*!< Buffer pool size */
  ulint innodb_buffer_pool_pages_data;        /*!< Data pages */
  ulint innodb_buffer_pool_bytes_data;        /*!< File bytes used */
  ulint innodb_buffer_pool_pages_dirty;       /*!< Dirty data pages */
  ulint innodb_buffer_pool_bytes_dirty;       /*!< File bytes modified */
  ulint innodb_buffer_pool_pages_misc;        /*!< Miscellanous pages */
  ulint innodb_buffer_pool_pages_free;        /*!< Free pages */
#ifdef UNIV_DEBUG
  ulint innodb_buffer_pool_pages_latched;  /*!< Latched pages */
#endif                                     /* UNIV_DEBUG */
  ulint innodb_buffer_pool_read_requests;  /*!< buf_pool->stat.n_page_gets */
  ulint innodb_buffer_pool_reads;          /*!< srv_buf_pool_reads */
  ulint innodb_buffer_pool_wait_free;      /*!< srv_buf_pool_wait_free */
  ulint innodb_buffer_pool_pages_flushed;  /*!< srv_buf_pool_flushed */
  ulint innodb_buffer_pool_write_requests; /*!< srv_buf_pool_write_requests */
  ulint innodb_buffer_pool_read_ahead_rnd; /*!< srv_read_ahead_rnd */
  ulint innodb_buffer_pool_read_ahead;     /*!< srv_read_ahead */
  ulint innodb_buffer_pool_read_ahead_evicted; /*!< srv_read_ahead evicted*/
  ulint innodb_dblwr_pages_written;            /*!< srv_dblwr_pages_written */
  ulint innodb_dblwr_writes;                   /*!< srv_dblwr_writes */
  ulint innodb_log_waits;                      /*!< srv_log_waits */
  ulint innodb_log_write_requests;             /*!< srv_log_write_requests */
  ulint innodb_log_writes;                     /*!< srv_log_writes */
  lsn_t innodb_os_log_written;                 /*!< srv_os_log_written */
  ulint innodb_os_log_fsyncs;                  /*!< fil_n_log_flushes */
  ulint innodb_os_log_pending_writes;          /*!< srv_os_log_pending_writes */
  ulint innodb_os_log_pending_fsyncs;          /*!< fil_n_pending_log_flushes */
  ulint innodb_page_size;                      /*!< UNIV_PAGE_SIZE */
  ulint innodb_pages_created;             /*!< buf_pool->stat.n_pages_created */
  ulint innodb_pages_read;                /*!< buf_pool->stat.n_pages_read */
  ulint innodb_pages_written;             /*!< buf_pool->stat.n_pages_written */
  ulint innodb_row_lock_waits;            /*!< srv_n_lock_wait_count */
  ulint innodb_row_lock_current_waits;    /*!< srv_n_lock_wait_current_count */
  int64_t innodb_row_lock_time;           /*!< srv_n_lock_wait_time
                                          / 1000 */
  ulint innodb_row_lock_time_avg;         /*!< srv_n_lock_wait_time
                                          / 1000
                                          / srv_n_lock_wait_count */
  ulint innodb_row_lock_time_max;         /*!< srv_n_lock_max_wait_time
                                          / 1000 */
  ulint innodb_rows_read;                 /*!< srv_n_rows_read */
  ulint innodb_rows_inserted;             /*!< srv_n_rows_inserted */
  ulint innodb_rows_updated;              /*!< srv_n_rows_updated */
  ulint innodb_rows_deleted;              /*!< srv_n_rows_deleted */
  ulint innodb_num_open_files;            /*!< fil_n_file_opened */
  ulint innodb_truncated_status_writes;   /*!< srv_truncated_status_writes */
  ulint innodb_undo_tablespaces_total;    /*!< total number of undo tablespaces
                                          innoDB is tracking. */
  ulint innodb_undo_tablespaces_implicit; /*!< number of undo tablespaces
                                          innoDB created implicitly. */
  ulint innodb_undo_tablespaces_explicit; /*!< number of undo tablespaces
                                          the dba created explicitly. */
  ulint innodb_undo_tablespaces_active;   /*!< number of active undo
                                          tablespaces */
#ifdef UNIV_DEBUG
  ulint innodb_purge_trx_id_age;      /*!< rw_max_trx_id - purged trx_id */
  ulint innodb_purge_view_trx_id_age; /*!< rw_max_trx_id
                                      - purged view's min trx_id */
  ulint innodb_ahi_drop_lookups;      /*!< number of adaptive hash
                                      index lookups when freeing
                                      file pages */
#endif                                /* UNIV_DEBUG */
};

#ifndef UNIV_HOTBACKUP
/** Thread slot in the thread table.  */
struct srv_slot_t {
  srv_thread_type type;   /*!< thread type: user,
                          utility etc. */
  ibool in_use;           /*!< TRUE if this slot
                          is in use */
  ibool suspended;        /*!< TRUE if the thread is
                          waiting for the event of this
                          slot */
  ib_time_t suspend_time; /*!< time when the thread was
                          suspended. Initialized by
                          lock_wait_table_reserve_slot()
                          for lock wait */
  ulong wait_timeout;     /*!< wait time that if exceeded
                          the thread will be timed out.
                          Initialized by
                          lock_wait_table_reserve_slot()
                          for lock wait */
  os_event_t event;       /*!< event used in suspending
                          the thread when it has nothing
                          to do */
  que_thr_t *thr;         /*!< suspended query thread
                          (only used for user threads) */
};
#endif /* !UNIV_HOTBACKUP */

#endif
