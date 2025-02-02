#pragma once

#include <innodb/univ/univ.h>

#include <innodb/pfs/mysql_pfs_key_t.h>

#include <innodb/pfs/PFS_NOT_INSTRUMENTED.h>


#if defined UNIV_PFS_MUTEX || defined UNIV_PFS_RWLOCK

/* By default, buffer mutexes and rwlocks will be excluded from
instrumentation due to their large number of instances. */
#define PFS_SKIP_BUFFER_MUTEX_RWLOCK

/* By default, event->mutex will also be excluded from instrumentation */
#define PFS_SKIP_EVENT_MUTEX

#endif /* UNIV_PFS_MUTEX || UNIV_PFS_RWLOCK */





#ifdef UNIV_PFS_MUTEX
/* Key defines to register InnoDB mutexes with performance schema */
extern mysql_pfs_key_t autoinc_mutex_key;
extern mysql_pfs_key_t autoinc_persisted_mutex_key;
#ifndef PFS_SKIP_BUFFER_MUTEX_RWLOCK
extern mysql_pfs_key_t buffer_block_mutex_key;
#endif /* !PFS_SKIP_BUFFER_MUTEX_RWLOCK */
extern mysql_pfs_key_t buf_pool_chunks_mutex_key;
extern mysql_pfs_key_t buf_pool_flush_state_mutex_key;
extern mysql_pfs_key_t buf_pool_LRU_list_mutex_key;
extern mysql_pfs_key_t buf_pool_free_list_mutex_key;
extern mysql_pfs_key_t buf_pool_zip_free_mutex_key;
extern mysql_pfs_key_t buf_pool_zip_hash_mutex_key;
extern mysql_pfs_key_t buf_pool_zip_mutex_key;
extern mysql_pfs_key_t cache_last_read_mutex_key;
extern mysql_pfs_key_t dict_foreign_err_mutex_key;
extern mysql_pfs_key_t dict_persist_dirty_tables_mutex_key;
extern mysql_pfs_key_t dict_sys_mutex_key;
extern mysql_pfs_key_t dict_table_mutex_key;
extern mysql_pfs_key_t parser_mutex_key;
extern mysql_pfs_key_t fil_system_mutex_key;
extern mysql_pfs_key_t flush_list_mutex_key;
extern mysql_pfs_key_t fts_bg_threads_mutex_key;
extern mysql_pfs_key_t fts_delete_mutex_key;
extern mysql_pfs_key_t fts_optimize_mutex_key;
extern mysql_pfs_key_t fts_doc_id_mutex_key;
extern mysql_pfs_key_t fts_pll_tokenize_mutex_key;
extern mysql_pfs_key_t hash_table_mutex_key;
extern mysql_pfs_key_t ibuf_bitmap_mutex_key;
extern mysql_pfs_key_t ibuf_mutex_key;
extern mysql_pfs_key_t ibuf_pessimistic_insert_mutex_key;
extern mysql_pfs_key_t lock_free_hash_mutex_key;
extern mysql_pfs_key_t page_sys_arch_mutex_key;
extern mysql_pfs_key_t page_sys_arch_oper_mutex_key;
extern mysql_pfs_key_t page_sys_arch_client_mutex_key;
extern mysql_pfs_key_t mutex_list_mutex_key;
extern mysql_pfs_key_t recalc_pool_mutex_key;
extern mysql_pfs_key_t page_cleaner_mutex_key;
extern mysql_pfs_key_t purge_sys_pq_mutex_key;
extern mysql_pfs_key_t recv_sys_mutex_key;
extern mysql_pfs_key_t recv_writer_mutex_key;
extern mysql_pfs_key_t rtr_active_mutex_key;
extern mysql_pfs_key_t rtr_match_mutex_key;
extern mysql_pfs_key_t rtr_path_mutex_key;
extern mysql_pfs_key_t rtr_ssn_mutex_key;
extern mysql_pfs_key_t temp_space_rseg_mutex_key;
extern mysql_pfs_key_t undo_space_rseg_mutex_key;
extern mysql_pfs_key_t trx_sys_rseg_mutex_key;
extern mysql_pfs_key_t page_zip_stat_per_index_mutex_key;
#ifdef UNIV_DEBUG
extern mysql_pfs_key_t rw_lock_debug_mutex_key;
#endif /* UNIV_DEBUG */
extern mysql_pfs_key_t rw_lock_list_mutex_key;
extern mysql_pfs_key_t rw_lock_mutex_key;
extern mysql_pfs_key_t srv_dict_tmpfile_mutex_key;
extern mysql_pfs_key_t srv_innodb_monitor_mutex_key;
extern mysql_pfs_key_t srv_misc_tmpfile_mutex_key;
extern mysql_pfs_key_t srv_monitor_file_mutex_key;
#ifdef UNIV_DEBUG
extern mysql_pfs_key_t sync_thread_mutex_key;
#endif /* UNIV_DEBUG */
extern mysql_pfs_key_t buf_dblwr_mutex_key;
extern mysql_pfs_key_t trx_undo_mutex_key;
extern mysql_pfs_key_t trx_mutex_key;
extern mysql_pfs_key_t trx_pool_mutex_key;
extern mysql_pfs_key_t trx_pool_manager_mutex_key;
extern mysql_pfs_key_t temp_pool_manager_mutex_key;
extern mysql_pfs_key_t lock_mutex_key;
extern mysql_pfs_key_t lock_wait_mutex_key;
extern mysql_pfs_key_t trx_sys_mutex_key;
extern mysql_pfs_key_t srv_sys_mutex_key;
extern mysql_pfs_key_t srv_threads_mutex_key;
#ifndef PFS_SKIP_EVENT_MUTEX
extern mysql_pfs_key_t event_mutex_key;
extern mysql_pfs_key_t event_manager_mutex_key;
#endif /* !PFS_SKIP_EVENT_MUTEX */
extern mysql_pfs_key_t sync_array_mutex_key;
extern mysql_pfs_key_t zip_pad_mutex_key;
extern mysql_pfs_key_t row_drop_list_mutex_key;
extern mysql_pfs_key_t file_open_mutex_key;
extern mysql_pfs_key_t master_key_id_mutex_key;
extern mysql_pfs_key_t clone_sys_mutex_key;
extern mysql_pfs_key_t clone_task_mutex_key;
extern mysql_pfs_key_t clone_snapshot_mutex_key;
#endif /* UNIV_PFS_MUTEX */

#ifdef UNIV_PFS_RWLOCK
/* Following are rwlock keys used to register with MySQL
performance schema */
extern mysql_pfs_key_t btr_search_latch_key;
#ifndef PFS_SKIP_BUFFER_MUTEX_RWLOCK
extern mysql_pfs_key_t buf_block_lock_key;
#endif /* !PFS_SKIP_BUFFER_MUTEX_RWLOCK */
#ifdef UNIV_DEBUG
extern mysql_pfs_key_t buf_block_debug_latch_key;
#endif /* UNIV_DEBUG */
extern mysql_pfs_key_t dict_operation_lock_key;
extern mysql_pfs_key_t undo_spaces_lock_key;
extern mysql_pfs_key_t rsegs_lock_key;
extern mysql_pfs_key_t fil_space_latch_key;
extern mysql_pfs_key_t fts_cache_rw_lock_key;
extern mysql_pfs_key_t fts_cache_init_rw_lock_key;
extern mysql_pfs_key_t trx_i_s_cache_lock_key;
extern mysql_pfs_key_t trx_purge_latch_key;
extern mysql_pfs_key_t index_tree_rw_lock_key;
extern mysql_pfs_key_t index_online_log_key;
extern mysql_pfs_key_t dict_table_stats_key;
extern mysql_pfs_key_t trx_sys_rw_lock_key;
extern mysql_pfs_key_t hash_table_locks_key;
#endif /* UNIV_PFS_RWLOCK */


