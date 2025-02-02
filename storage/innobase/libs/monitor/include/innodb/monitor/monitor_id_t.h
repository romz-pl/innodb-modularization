#pragma once

#include <innodb/univ/univ.h>

/** This enumeration defines internal monitor identifier used internally
to identify each particular counter. Its value indexes into two arrays,
one is the "innodb_counter_value" array which records actual monitor
counter values, the other is "innodb_counter_info" array which describes
each counter's basic information (name, desc etc.). A couple of
naming rules here:
1) If the monitor defines a module, it starts with MONITOR_MODULE
2) If the monitor uses exisitng counters from "status variable", its ID
name shall start with MONITOR_OVLD

Please refer to "innodb_counter_info" in srv/srv0mon.cc for detail
information for each monitor counter */

enum monitor_id_t {
  /* This is to identify the default value set by the metrics
  control global variables */
  MONITOR_DEFAULT_START = 0,

  /* Start of Metadata counter */
  MONITOR_MODULE_METADATA,
  MONITOR_TABLE_OPEN,
  MONITOR_TABLE_CLOSE,
  MONITOR_TABLE_REFERENCE,

  /* Lock manager related counters */
  MONITOR_MODULE_LOCK,
  MONITOR_DEADLOCK,
  MONITOR_TIMEOUT,
  MONITOR_LOCKREC_WAIT,
  MONITOR_TABLELOCK_WAIT,
  MONITOR_NUM_RECLOCK_REQ,
  MONITOR_RECLOCK_CREATED,
  MONITOR_RECLOCK_REMOVED,
  MONITOR_NUM_RECLOCK,
  MONITOR_TABLELOCK_CREATED,
  MONITOR_TABLELOCK_REMOVED,
  MONITOR_NUM_TABLELOCK,
  MONITOR_OVLD_ROW_LOCK_CURRENT_WAIT,
  MONITOR_OVLD_LOCK_WAIT_TIME,
  MONITOR_OVLD_LOCK_MAX_WAIT_TIME,
  MONITOR_OVLD_ROW_LOCK_WAIT,
  MONITOR_OVLD_LOCK_AVG_WAIT_TIME,

  /* Buffer and I/O realted counters. */
  MONITOR_MODULE_BUFFER,
  MONITOR_OVLD_BUFFER_POOL_SIZE,
  MONITOR_OVLD_BUF_POOL_READS,
  MONITOR_OVLD_BUF_POOL_READ_REQUESTS,
  MONITOR_OVLD_BUF_POOL_WRITE_REQUEST,
  MONITOR_OVLD_BUF_POOL_WAIT_FREE,
  MONITOR_OVLD_BUF_POOL_READ_AHEAD,
  MONITOR_OVLD_BUF_POOL_READ_AHEAD_EVICTED,
  MONITOR_OVLD_BUF_POOL_PAGE_TOTAL,
  MONITOR_OVLD_BUF_POOL_PAGE_MISC,
  MONITOR_OVLD_BUF_POOL_PAGES_DATA,
  MONITOR_OVLD_BUF_POOL_BYTES_DATA,
  MONITOR_OVLD_BUF_POOL_PAGES_DIRTY,
  MONITOR_OVLD_BUF_POOL_BYTES_DIRTY,
  MONITOR_OVLD_BUF_POOL_PAGES_FREE,
  MONITOR_OVLD_PAGE_CREATED,
  MONITOR_OVLD_PAGES_WRITTEN,
  MONITOR_OVLD_PAGES_READ,
  MONITOR_OVLD_BYTE_READ,
  MONITOR_OVLD_BYTE_WRITTEN,
  MONITOR_FLUSH_BATCH_SCANNED,
  MONITOR_FLUSH_BATCH_SCANNED_NUM_CALL,
  MONITOR_FLUSH_BATCH_SCANNED_PER_CALL,
  MONITOR_FLUSH_BATCH_TOTAL_PAGE,
  MONITOR_FLUSH_BATCH_COUNT,
  MONITOR_FLUSH_BATCH_PAGES,
  MONITOR_FLUSH_NEIGHBOR_TOTAL_PAGE,
  MONITOR_FLUSH_NEIGHBOR_COUNT,
  MONITOR_FLUSH_NEIGHBOR_PAGES,
  MONITOR_FLUSH_N_TO_FLUSH_REQUESTED,

  MONITOR_FLUSH_N_TO_FLUSH_BY_AGE,
  MONITOR_FLUSH_ADAPTIVE_AVG_TIME_SLOT,
  MONITOR_LRU_BATCH_FLUSH_AVG_TIME_SLOT,

  MONITOR_FLUSH_ADAPTIVE_AVG_TIME_THREAD,
  MONITOR_LRU_BATCH_FLUSH_AVG_TIME_THREAD,
  MONITOR_FLUSH_ADAPTIVE_AVG_TIME_EST,
  MONITOR_LRU_BATCH_FLUSH_AVG_TIME_EST,
  MONITOR_FLUSH_AVG_TIME,

  MONITOR_FLUSH_ADAPTIVE_AVG_PASS,
  MONITOR_LRU_BATCH_FLUSH_AVG_PASS,
  MONITOR_FLUSH_AVG_PASS,

  MONITOR_LRU_GET_FREE_LOOPS,
  MONITOR_LRU_GET_FREE_WAITS,

  MONITOR_FLUSH_AVG_PAGE_RATE,
  MONITOR_FLUSH_LSN_AVG_RATE,
  MONITOR_FLUSH_PCT_FOR_DIRTY,
  MONITOR_FLUSH_PCT_FOR_LSN,
  MONITOR_FLUSH_SYNC_WAITS,
  MONITOR_FLUSH_ADAPTIVE_TOTAL_PAGE,
  MONITOR_FLUSH_ADAPTIVE_COUNT,
  MONITOR_FLUSH_ADAPTIVE_PAGES,
  MONITOR_FLUSH_SYNC_TOTAL_PAGE,
  MONITOR_FLUSH_SYNC_COUNT,
  MONITOR_FLUSH_SYNC_PAGES,
  MONITOR_FLUSH_BACKGROUND_TOTAL_PAGE,
  MONITOR_FLUSH_BACKGROUND_COUNT,
  MONITOR_FLUSH_BACKGROUND_PAGES,
  MONITOR_LRU_BATCH_SCANNED,
  MONITOR_LRU_BATCH_SCANNED_NUM_CALL,
  MONITOR_LRU_BATCH_SCANNED_PER_CALL,
  MONITOR_LRU_BATCH_FLUSH_TOTAL_PAGE,
  MONITOR_LRU_BATCH_FLUSH_COUNT,
  MONITOR_LRU_BATCH_FLUSH_PAGES,
  MONITOR_LRU_BATCH_EVICT_TOTAL_PAGE,
  MONITOR_LRU_BATCH_EVICT_COUNT,
  MONITOR_LRU_BATCH_EVICT_PAGES,
  MONITOR_LRU_SINGLE_FLUSH_SCANNED,
  MONITOR_LRU_SINGLE_FLUSH_SCANNED_NUM_CALL,
  MONITOR_LRU_SINGLE_FLUSH_SCANNED_PER_CALL,
  MONITOR_LRU_SINGLE_FLUSH_FAILURE_COUNT,
  MONITOR_LRU_GET_FREE_SEARCH,
  MONITOR_LRU_SEARCH_SCANNED,
  MONITOR_LRU_SEARCH_SCANNED_NUM_CALL,
  MONITOR_LRU_SEARCH_SCANNED_PER_CALL,
  MONITOR_LRU_UNZIP_SEARCH_SCANNED,
  MONITOR_LRU_UNZIP_SEARCH_SCANNED_NUM_CALL,
  MONITOR_LRU_UNZIP_SEARCH_SCANNED_PER_CALL,

  /* Buffer Page I/O specific counters. */
  MONITOR_MODULE_BUF_PAGE,
  MONITOR_INDEX_LEAF_PAGE_READ,
  MONITOR_INDEX_NON_LEAF_PAGE_READ,
  MONITOR_INDEX_IBUF_LEAF_PAGE_READ,
  MONITOR_INDEX_IBUF_NON_LEAF_PAGE_READ,
  MONITOR_UNDO_LOG_PAGE_READ,
  MONITOR_INODE_PAGE_READ,
  MONITOR_IBUF_FREELIST_PAGE_READ,
  MONITOR_IBUF_BITMAP_PAGE_READ,
  MONITOR_SYSTEM_PAGE_READ,
  MONITOR_TRX_SYSTEM_PAGE_READ,
  MONITOR_FSP_HDR_PAGE_READ,
  MONITOR_XDES_PAGE_READ,
  MONITOR_BLOB_PAGE_READ,
  MONITOR_ZBLOB_PAGE_READ,
  MONITOR_ZBLOB2_PAGE_READ,
  MONITOR_RSEG_ARRAY_PAGE_READ,
  MONITOR_OTHER_PAGE_READ,
  MONITOR_INDEX_LEAF_PAGE_WRITTEN,
  MONITOR_INDEX_NON_LEAF_PAGE_WRITTEN,
  MONITOR_INDEX_IBUF_LEAF_PAGE_WRITTEN,
  MONITOR_INDEX_IBUF_NON_LEAF_PAGE_WRITTEN,
  MONITOR_UNDO_LOG_PAGE_WRITTEN,
  MONITOR_INODE_PAGE_WRITTEN,
  MONITOR_IBUF_FREELIST_PAGE_WRITTEN,
  MONITOR_IBUF_BITMAP_PAGE_WRITTEN,
  MONITOR_SYSTEM_PAGE_WRITTEN,
  MONITOR_TRX_SYSTEM_PAGE_WRITTEN,
  MONITOR_FSP_HDR_PAGE_WRITTEN,
  MONITOR_XDES_PAGE_WRITTEN,
  MONITOR_BLOB_PAGE_WRITTEN,
  MONITOR_ZBLOB_PAGE_WRITTEN,
  MONITOR_ZBLOB2_PAGE_WRITTEN,
  MONITOR_RSEG_ARRAY_PAGE_WRITTEN,
  MONITOR_OTHER_PAGE_WRITTEN,
  MONITOR_ON_LOG_NO_WAITS_PAGE_WRITTEN,
  MONITOR_ON_LOG_WAITS_PAGE_WRITTEN,
  MONITOR_ON_LOG_WAIT_LOOPS_PAGE_WRITTEN,

  /* OS level counters (I/O) */
  MONITOR_MODULE_OS,
  MONITOR_OVLD_OS_FILE_READ,
  MONITOR_OVLD_OS_FILE_WRITE,
  MONITOR_OVLD_OS_FSYNC,
  MONITOR_OS_PENDING_READS,
  MONITOR_OS_PENDING_WRITES,
  MONITOR_OVLD_OS_LOG_WRITTEN,
  MONITOR_OVLD_OS_LOG_FSYNC,
  MONITOR_OVLD_OS_LOG_PENDING_FSYNC,
  MONITOR_OVLD_OS_LOG_PENDING_WRITES,

  /* Transaction related counters */
  MONITOR_MODULE_TRX,
  MONITOR_TRX_RW_COMMIT,
  MONITOR_TRX_RO_COMMIT,
  MONITOR_TRX_NL_RO_COMMIT,
  MONITOR_TRX_COMMIT_UNDO,
  MONITOR_TRX_ROLLBACK,
  MONITOR_TRX_ROLLBACK_SAVEPOINT,
  MONITOR_TRX_ROLLBACK_ACTIVE,
  MONITOR_TRX_ACTIVE,
  MONITOR_TRX_ON_LOG_NO_WAITS,
  MONITOR_TRX_ON_LOG_WAITS,
  MONITOR_TRX_ON_LOG_WAIT_LOOPS,
  MONITOR_RSEG_HISTORY_LEN,
  MONITOR_NUM_UNDO_SLOT_USED,
  MONITOR_NUM_UNDO_SLOT_CACHED,
  MONITOR_RSEG_CUR_SIZE,

  /* Purge related counters */
  MONITOR_MODULE_PURGE,
  MONITOR_N_DEL_ROW_PURGE,
  MONITOR_N_UPD_EXIST_EXTERN,
  MONITOR_PURGE_INVOKED,
  MONITOR_PURGE_N_PAGE_HANDLED,
  MONITOR_DML_PURGE_DELAY,
  MONITOR_PURGE_STOP_COUNT,
  MONITOR_PURGE_RESUME_COUNT,
  MONITOR_PURGE_TRUNCATE_HISTORY_COUNT,
  MONITOR_PURGE_TRUNCATE_HISTORY_MICROSECOND,

  /* Undo tablespace truncation */
  MONITOR_UNDO_TRUNCATE,
  MONITOR_UNDO_TRUNCATE_COUNT,
  MONITOR_UNDO_TRUNCATE_SWEEP_COUNT,
  MONITOR_UNDO_TRUNCATE_SWEEP_MICROSECOND,
  MONITOR_UNDO_TRUNCATE_START_LOGGING_COUNT,
  MONITOR_UNDO_TRUNCATE_FLUSH_COUNT,
  MONITOR_UNDO_TRUNCATE_FLUSH_MICROSECOND,
  MONITOR_UNDO_TRUNCATE_DONE_LOGGING_COUNT,
  MONITOR_UNDO_TRUNCATE_MICROSECOND,

  /* Recovery related counters */
  MONITOR_MODULE_REDO_LOG,
  MONITOR_OVLD_LSN_FLUSHDISK,
  MONITOR_OVLD_LSN_CHECKPOINT,
  MONITOR_OVLD_LSN_CURRENT,
  MONITOR_OVLD_LSN_ARCHIVED,
  MONITOR_OVLD_LSN_CHECKPOINT_AGE,
  MONITOR_OVLD_LSN_BUF_DIRTY_PAGES_ADDED,
  MONITOR_OVLD_BUF_OLDEST_LSN_APPROX,
  MONITOR_OVLD_BUF_OLDEST_LSN_LWM,
  MONITOR_OVLD_MAX_AGE_ASYNC,
  MONITOR_OVLD_MAX_AGE_SYNC,
  MONITOR_OVLD_LOG_WAITS,
  MONITOR_OVLD_LOG_WRITE_REQUEST,
  MONITOR_OVLD_LOG_WRITES,

  MONITOR_LOG_FLUSH_TOTAL_TIME,
  MONITOR_LOG_FLUSH_MAX_TIME,
  MONITOR_LOG_FLUSH_AVG_TIME,
  MONITOR_LOG_FLUSH_LSN_AVG_RATE,

  MONITOR_LOG_FULL_BLOCK_WRITES,
  MONITOR_LOG_PARTIAL_BLOCK_WRITES,
  MONITOR_LOG_PADDED,
  MONITOR_LOG_NEXT_FILE,
  MONITOR_LOG_CHECKPOINTS,
  MONITOR_LOG_FREE_SPACE,
  MONITOR_LOG_CONCURRENCY_MARGIN,

  MONITOR_LOG_WRITER_NO_WAITS,
  MONITOR_LOG_WRITER_WAITS,
  MONITOR_LOG_WRITER_WAIT_LOOPS,
  MONITOR_LOG_WRITER_ON_FREE_SPACE_WAITS,
  MONITOR_LOG_WRITER_ON_ARCHIVER_WAITS,

  MONITOR_LOG_FLUSHER_NO_WAITS,
  MONITOR_LOG_FLUSHER_WAITS,
  MONITOR_LOG_FLUSHER_WAIT_LOOPS,

  MONITOR_LOG_WRITE_NOTIFIER_NO_WAITS,
  MONITOR_LOG_WRITE_NOTIFIER_WAITS,
  MONITOR_LOG_WRITE_NOTIFIER_WAIT_LOOPS,

  MONITOR_LOG_FLUSH_NOTIFIER_NO_WAITS,
  MONITOR_LOG_FLUSH_NOTIFIER_WAITS,
  MONITOR_LOG_FLUSH_NOTIFIER_WAIT_LOOPS,

  MONITOR_LOG_WRITE_TO_FILE_REQUESTS_INTERVAL,

  MONITOR_LOG_ON_WRITE_NO_WAITS,
  MONITOR_LOG_ON_WRITE_WAITS,
  MONITOR_LOG_ON_WRITE_WAIT_LOOPS,
  MONITOR_LOG_ON_FLUSH_NO_WAITS,
  MONITOR_LOG_ON_FLUSH_WAITS,
  MONITOR_LOG_ON_FLUSH_WAIT_LOOPS,
  MONITOR_LOG_ON_RECENT_WRITTEN_WAIT_LOOPS,
  MONITOR_LOG_ON_RECENT_CLOSED_WAIT_LOOPS,
  MONITOR_LOG_ON_BUFFER_SPACE_NO_WAITS,
  MONITOR_LOG_ON_BUFFER_SPACE_WAITS,
  MONITOR_LOG_ON_BUFFER_SPACE_WAIT_LOOPS,
  MONITOR_LOG_ON_FILE_SPACE_NO_WAITS,
  MONITOR_LOG_ON_FILE_SPACE_WAITS,
  MONITOR_LOG_ON_FILE_SPACE_WAIT_LOOPS,

  /* Page Manager related counters */
  MONITOR_MODULE_PAGE,
  MONITOR_PAGE_COMPRESS,
  MONITOR_PAGE_DECOMPRESS,
  MONITOR_PAD_INCREMENTS,
  MONITOR_PAD_DECREMENTS,

  /* Index related counters */
  MONITOR_MODULE_INDEX,
  MONITOR_INDEX_SPLIT,
  MONITOR_INDEX_MERGE_ATTEMPTS,
  MONITOR_INDEX_MERGE_SUCCESSFUL,
  MONITOR_INDEX_REORG_ATTEMPTS,
  MONITOR_INDEX_REORG_SUCCESSFUL,
  MONITOR_INDEX_DISCARD,

  /* Adaptive Hash Index related counters */
  MONITOR_MODULE_ADAPTIVE_HASH,
  MONITOR_OVLD_ADAPTIVE_HASH_SEARCH,
  MONITOR_OVLD_ADAPTIVE_HASH_SEARCH_BTREE,
  MONITOR_ADAPTIVE_HASH_PAGE_ADDED,
  MONITOR_ADAPTIVE_HASH_PAGE_REMOVED,
  MONITOR_ADAPTIVE_HASH_ROW_ADDED,
  MONITOR_ADAPTIVE_HASH_ROW_REMOVED,
  MONITOR_ADAPTIVE_HASH_ROW_REMOVE_NOT_FOUND,
  MONITOR_ADAPTIVE_HASH_ROW_UPDATED,

  /* Tablespace related counters */
  MONITOR_MODULE_FIL_SYSTEM,
  MONITOR_OVLD_N_FILE_OPENED,

  /* InnoDB Change Buffer related counters */
  MONITOR_MODULE_IBUF_SYSTEM,
  MONITOR_OVLD_IBUF_MERGE_INSERT,
  MONITOR_OVLD_IBUF_MERGE_DELETE,
  MONITOR_OVLD_IBUF_MERGE_PURGE,
  MONITOR_OVLD_IBUF_MERGE_DISCARD_INSERT,
  MONITOR_OVLD_IBUF_MERGE_DISCARD_DELETE,
  MONITOR_OVLD_IBUF_MERGE_DISCARD_PURGE,
  MONITOR_OVLD_IBUF_MERGES,
  MONITOR_OVLD_IBUF_SIZE,

  /* Counters for server operations */
  MONITOR_MODULE_SERVER,
  MONITOR_MASTER_THREAD_SLEEP,
  MONITOR_OVLD_SERVER_ACTIVITY,
  MONITOR_MASTER_ACTIVE_LOOPS,
  MONITOR_MASTER_IDLE_LOOPS,
  MONITOR_SRV_BACKGROUND_DROP_TABLE_MICROSECOND,
  MONITOR_SRV_IBUF_MERGE_MICROSECOND,
  MONITOR_SRV_MEM_VALIDATE_MICROSECOND,
  MONITOR_SRV_PURGE_MICROSECOND,
  MONITOR_SRV_DICT_LRU_MICROSECOND,
  MONITOR_SRV_DICT_LRU_EVICT_COUNT,
  MONITOR_OVLD_SRV_DBLWR_WRITES,
  MONITOR_OVLD_SRV_DBLWR_PAGES_WRITTEN,
  MONITOR_OVLD_SRV_PAGE_SIZE,
  MONITOR_OVLD_RWLOCK_S_SPIN_WAITS,
  MONITOR_OVLD_RWLOCK_X_SPIN_WAITS,
  MONITOR_OVLD_RWLOCK_SX_SPIN_WAITS,
  MONITOR_OVLD_RWLOCK_S_SPIN_ROUNDS,
  MONITOR_OVLD_RWLOCK_X_SPIN_ROUNDS,
  MONITOR_OVLD_RWLOCK_SX_SPIN_ROUNDS,
  MONITOR_OVLD_RWLOCK_S_OS_WAITS,
  MONITOR_OVLD_RWLOCK_X_OS_WAITS,
  MONITOR_OVLD_RWLOCK_SX_OS_WAITS,

  /* Data DML related counters */
  MONITOR_MODULE_DML_STATS,
  MONITOR_OLVD_ROW_READ,
  MONITOR_OLVD_ROW_INSERTED,
  MONITOR_OLVD_ROW_DELETED,
  MONITOR_OLVD_ROW_UPDTATED,

  /* Data DDL related counters */
  MONITOR_MODULE_DDL_STATS,
  MONITOR_BACKGROUND_DROP_TABLE,
  MONITOR_ONLINE_CREATE_INDEX,
  MONITOR_PENDING_ALTER_TABLE,
  MONITOR_ALTER_TABLE_SORT_FILES,
  MONITOR_ALTER_TABLE_LOG_FILES,

  MONITOR_MODULE_ICP,
  MONITOR_ICP_ATTEMPTS,
  MONITOR_ICP_NO_MATCH,
  MONITOR_ICP_OUT_OF_RANGE,
  MONITOR_ICP_MATCH,

  /* Mutex/RW-Lock related counters */
  MONITOR_MODULE_LATCHES,
  MONITOR_LATCHES,

  /* CPU usage information */
  MONITOR_MODULE_CPU,
  MONITOR_CPU_UTIME_ABS,
  MONITOR_CPU_STIME_ABS,
  MONITOR_CPU_UTIME_PCT,
  MONITOR_CPU_STIME_PCT,
  MONITOR_CPU_N,

  MONITOR_MODULE_PAGE_TRACK,
  MONITOR_PAGE_TRACK_RESETS,
  MONITOR_PAGE_TRACK_PARTIAL_BLOCK_WRITES,
  MONITOR_PAGE_TRACK_FULL_BLOCK_WRITES,
  MONITOR_PAGE_TRACK_CHECKPOINT_PARTIAL_FLUSH_REQUEST,

  /* This is used only for control system to turn
  on/off and reset all monitor counters */
  MONITOR_ALL_COUNTER,

  /* This must be the last member */
  NUM_MONITOR
};

/** This informs the monitor control system to turn
on/off and reset monitor counters through wild card match */
#define MONITOR_WILDCARD_MATCH (NUM_MONITOR + 1)

/** Cannot find monitor counter with a specified name */
#define MONITOR_NO_MATCH (NUM_MONITOR + 2)
