#pragma once

#include <innodb/univ/univ.h>


#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/dict_mem/table_lock_list_t.h>
#include <innodb/dict_types/table_name_t.h>
#include <innodb/dict_types/id_name_t.h>
#include <innodb/hash/hash_node_t.h>
#include <innodb/dict_mem/os_once.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/dict_mem/table_dirty_status.h>
#include <innodb/dict_mem/dict_foreign_set.h>
#include <innodb/time/ib_time_t.h>
#include <innodb/dict_types/ib_quiesce_t.h>
#include <innodb/dict_mem/temp_prebuilt_vec.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/dict_mem/dict_s_col_list.h>

#include "sql/dd/object_id.h"


struct dict_vcol_templ_t;
struct dict_v_col_t;
struct dict_foreign_t;
class fts_t;

/** Data structure for a database table.  Most fields will be
initialized to 0, NULL or FALSE in dict_mem_table_create(). */
struct dict_table_t {
  /** Check if the table is compressed.
  @return true if compressed, false otherwise. */
  bool is_compressed() const { return (DICT_TF_GET_ZIP_SSIZE(flags) != 0); }

  /** Get reference count.
  @return current value of n_ref_count */
  uint64_t get_ref_count() const;

  /** Acquire the table handle. */
  void acquire();

  /** Acquire the table handle, with lock() and unlock() the table.
  This function needs to be called for opening table when the table
  is in memory and later the stats information would be initialized */
  void acquire_with_lock();

  /** Release the table handle. */
  void release();

  /** Lock the table handle. */
  void lock();

  /** Unlock the table handle. */
  void unlock();

#ifndef UNIV_HOTBACKUP
  /** Mutex of the table for concurrency access. */
  ib_mutex_t *mutex;

  /** Creation state of mutex. */
  volatile os_once::state_t mutex_created;
#endif /* !UNIV_HOTBACKUP */

  /** Id of the table. */
  table_id_t id;

  /** Memory heap. If you allocate from this heap after the table has
  been created then be sure to account the allocation into
  dict_sys->size. When closing the table we do something like
  dict_sys->size -= mem_heap_get_size(table->heap) and if that is going
  to become negative then we would assert. Something like this should do:
  old_size = mem_heap_get_size()
  mem_heap_alloc()
  new_size = mem_heap_get_size()
  dict_sys->size += new_size - old_size. */
  mem_heap_t *heap;

  /** Table name. */
  table_name_t name;

  /** Truncate name. */
  table_name_t trunc_name;

  /** NULL or the directory path specified by DATA DIRECTORY. */
  char *data_dir_path;

  /** NULL or the tablespace name that this table is assigned to,
  specified by the TABLESPACE option.*/
  id_name_t tablespace;

  /** Space where the clustered index of the table is placed. */
  space_id_t space;

  /** dd::Tablespace::id of the table */
  dd::Object_id dd_space_id;

  /** Stores information about:
  1 row format (redundant or compact),
  2 compressed page size (zip shift size),
  3 whether using atomic blobs,
  4 whether the table has been created with the option DATA DIRECTORY.
  Use DICT_TF_GET_COMPACT(), DICT_TF_GET_ZIP_SSIZE(),
  DICT_TF_HAS_ATOMIC_BLOBS() and DICT_TF_HAS_DATA_DIR() to parse this
  flag. */
  unsigned flags : DICT_TF_BITS;

  /** Stores information about:
  1 whether the table has been created using CREATE TEMPORARY TABLE,
  2 whether the table has an internally defined DOC ID column,
  3 whether the table has a FTS index,
  4 whether DOC ID column need to be added to the FTS index,
  5 whether the table is being created its own tablespace,
  6 whether the table has been DISCARDed,
  7 whether the aux FTS tables names are in hex.
  8 whether the table is instinc table.
  9 whether the table has encryption setting.
  Use DICT_TF2_FLAG_IS_SET() to parse this flag. */
  unsigned flags2 : DICT_TF2_BITS;

  /** TRUE if the table is an intermediate table during copy alter
  operation or a partition/subpartition which is required for copying
  data and skip the undo log for insertion of row in the table.
  This variable will be set and unset during extra(), or during the
  process of altering partitions */
  unsigned skip_alter_undo : 1;

  /** TRUE if this is in a single-table tablespace and the .ibd file is
  missing. Then we must return in ha_innodb.cc an error if the user
  tries to query such an orphaned table. */
  unsigned ibd_file_missing : 1;

  /** TRUE if the table object has been added to the dictionary cache. */
  unsigned cached : 1;

  /** TRUE if the table is to be dropped, but not yet actually dropped
  (could in the background drop list). It is turned on at the beginning
  of row_drop_table_for_mysql() and turned off just before we start to
  update system tables for the drop. It is protected by
  dict_operation_lock. */
  unsigned to_be_dropped : 1;

  /** Number of non-virtual columns defined so far. */
  unsigned n_def : 10;

  /** Number of non-virtual columns. */
  unsigned n_cols : 10;

  /** Number of non-virtual columns before first instant ADD COLUMN,
  including the system columns like n_cols. */
  unsigned n_instant_cols : 10;

  /** Number of total columns (inlcude virtual and non-virtual) */
  unsigned n_t_cols : 10;

  /** Number of total columns defined so far. */
  unsigned n_t_def : 10;

  /** Number of virtual columns defined so far. */
  unsigned n_v_def : 10;

  /** Number of virtual columns. */
  unsigned n_v_cols : 10;

  /** TRUE if this table is expected to be kept in memory. This table
  could be a table that has FK relationships or is undergoing DDL */
  unsigned can_be_evicted : 1;

  /** TRUE if this table is not evictable(can_be_evicted) and this is
  because of DDL operation */
  unsigned ddl_not_evictable : 1;

  /** TRUE if some indexes should be dropped after ONLINE_INDEX_ABORTED
  or ONLINE_INDEX_ABORTED_DROPPED. */
  unsigned drop_aborted : 1;

  /** Array of column descriptions. */
  dict_col_t *cols;

  /** Array of virtual column descriptions. */
  dict_v_col_t *v_cols;

  /** List of stored column descriptions. It is used only for foreign key
  check during create table and copy alter operations.
  During copy alter, s_cols list is filled during create table operation
  and need to preserve till rename table operation. That is the
  reason s_cols is a part of dict_table_t */
  dict_s_col_list *s_cols;

  /** Column names packed in a character string
  "name1\0name2\0...nameN\0". Until the string contains n_cols, it will
  be allocated from a temporary heap. The final string will be allocated
  from table->heap. */
  const char *col_names;

  /** Virtual column names */
  const char *v_col_names;

  /** Hash chain node. */
  hash_node_t name_hash;

  /** Hash chain node. */
  hash_node_t id_hash;

  /** The FTS_DOC_ID_INDEX, or NULL if no fulltext indexes exist */
  dict_index_t *fts_doc_id_index;

  /** List of indexes of the table. */
  UT_LIST_BASE_NODE_T(dict_index_t) indexes;

  /** List of foreign key constraints in the table. These refer to
  columns in other tables. */
  UT_LIST_BASE_NODE_T(dict_foreign_t) foreign_list;

  /** List of foreign key constraints which refer to this table. */
  UT_LIST_BASE_NODE_T(dict_foreign_t) referenced_list;

  /** Node of the LRU list of tables. */
  UT_LIST_NODE_T(dict_table_t) table_LRU;

  /** metadata version number of dd::Table::se_private_data() */
  uint64_t version;

  /** table dynamic metadata status, protected by dict_persist->mutex */
  std::atomic<table_dirty_status> dirty_status;

#ifndef UNIV_HOTBACKUP
  /** Node of the dirty table list of tables, which is protected
  by dict_persist->mutex */
  UT_LIST_NODE_T(dict_table_t) dirty_dict_tables;
#endif /* !UNIV_HOTBACKUP */

#ifdef UNIV_DEBUG
  /** This field is used to mark if a table is in the
  dirty_dict_tables_list. if the dirty_status is not of
  METADATA_CLEAN, the table should be in the list, otherwise not.
  This field should be protected by dict_persist->mutex too. */
  bool in_dirty_dict_tables_list;
#endif /* UNIV_DEBUG */

  /** Maximum recursive level we support when loading tables chained
  together with FK constraints. If exceeds this level, we will stop
  loading child table into memory along with its parent table. */
  unsigned fk_max_recusive_level : 8;

  /** Count of how many foreign key check operations are currently being
  performed on the table. We cannot drop the table while there are
  foreign key checks running on it. */
  ulint n_foreign_key_checks_running;

  /** Transaction id that last touched the table definition. Either when
  loading the definition or CREATE TABLE, or ALTER TABLE (prepare,
  commit, and rollback phases). */
  trx_id_t def_trx_id;

  /*!< set of foreign key constraints in the table; these refer to
  columns in other tables */
  dict_foreign_set foreign_set;

  /*!< set of foreign key constraints which refer to this table */
  dict_foreign_set referenced_set;

#ifdef UNIV_DEBUG
  /** This field is used to specify in simulations tables which are so
  big that disk should be accessed. Disk access is simulated by putting
  the thread to sleep for a while. NOTE that this flag is not stored to
  the data dictionary on disk, and the database will forget about value
  TRUE if it has to reload the table definition from disk. */
  ibool does_not_fit_in_memory;
#endif /* UNIV_DEBUG */

  /** TRUE if the maximum length of a single row exceeds BIG_ROW_SIZE.
  Initialized in dict_table_add_to_cache(). */
  unsigned big_rows : 1;

#ifndef UNIV_HOTBACKUP
  /** Statistics for query optimization. @{ */

  /** Creation state of 'stats_latch'. */
  volatile os_once::state_t stats_latch_created;

  /** This latch protects:
  "dict_table_t::stat_initialized",
  "dict_table_t::stat_n_rows (*)",
  "dict_table_t::stat_clustered_index_size",
  "dict_table_t::stat_sum_of_other_index_sizes",
  "dict_table_t::stat_modified_counter (*)",
  "dict_table_t::indexes*::stat_n_diff_key_vals[]",
  "dict_table_t::indexes*::stat_index_size",
  "dict_table_t::indexes*::stat_n_leaf_pages".
  (*) Those are not always protected for
  performance reasons. */
  rw_lock_t *stats_latch;

  /** TRUE if statistics have been calculated the first time after
  database startup or table creation. */
  unsigned stat_initialized : 1;

  /** Timestamp of last recalc of the stats. */
  ib_time_t stats_last_recalc;

/** The two bits below are set in the 'stat_persistent' member. They
have the following meaning:
1. _ON=0, _OFF=0, no explicit persistent stats setting for this table,
the value of the global srv_stats_persistent is used to determine
whether the table has persistent stats enabled or not
2. _ON=0, _OFF=1, persistent stats are explicitly disabled for this
table, regardless of the value of the global srv_stats_persistent
3. _ON=1, _OFF=0, persistent stats are explicitly enabled for this
table, regardless of the value of the global srv_stats_persistent
4. _ON=1, _OFF=1, not allowed, we assert if this ever happens. */
#define DICT_STATS_PERSISTENT_ON (1 << 1)
#define DICT_STATS_PERSISTENT_OFF (1 << 2)

  /** Indicates whether the table uses persistent stats or not. See
  DICT_STATS_PERSISTENT_ON and DICT_STATS_PERSISTENT_OFF. */
  ib_uint32_t stat_persistent;

/** The two bits below are set in the 'stats_auto_recalc' member. They
have the following meaning:
1. _ON=0, _OFF=0, no explicit auto recalc setting for this table, the
value of the global srv_stats_persistent_auto_recalc is used to
determine whether the table has auto recalc enabled or not
2. _ON=0, _OFF=1, auto recalc is explicitly disabled for this table,
regardless of the value of the global srv_stats_persistent_auto_recalc
3. _ON=1, _OFF=0, auto recalc is explicitly enabled for this table,
regardless of the value of the global srv_stats_persistent_auto_recalc
4. _ON=1, _OFF=1, not allowed, we assert if this ever happens. */
#define DICT_STATS_AUTO_RECALC_ON (1 << 1)
#define DICT_STATS_AUTO_RECALC_OFF (1 << 2)

  /** Indicates whether the table uses automatic recalc for persistent
  stats or not. See DICT_STATS_AUTO_RECALC_ON and
  DICT_STATS_AUTO_RECALC_OFF. */
  ib_uint32_t stats_auto_recalc;

  /** The number of pages to sample for this table during persistent
  stats estimation. If this is 0, then the value of the global
  srv_stats_persistent_sample_pages will be used instead. */
  ulint stats_sample_pages;

  /** Approximate number of rows in the table. We periodically calculate
  new estimates. */
  ib_uint64_t stat_n_rows;

  /** Approximate clustered index size in database pages. */
  ulint stat_clustered_index_size;

  /** Approximate size of other indexes in database pages. */
  ulint stat_sum_of_other_index_sizes;

  /** If FTS AUX table, parent table id */
  table_id_t parent_id;

  /** How many rows are modified since last stats recalc. When a row is
  inserted, updated, or deleted, we add 1 to this number; we calculate
  new estimates for the table and the indexes if the table has changed
  too much, see row_update_statistics_if_needed(). The counter is reset
  to zero at statistics calculation. This counter is not protected by
  any latch, because this is only used for heuristics. */
  ib_uint64_t stat_modified_counter;

/** Background stats thread is not working on this table. */
#define BG_STAT_NONE 0

/** Set in 'stats_bg_flag' when the background stats code is working
on this table. The DROP TABLE code waits for this to be cleared before
proceeding. */
#define BG_STAT_IN_PROGRESS (1 << 0)

/** Set in 'stats_bg_flag' when DROP TABLE starts waiting on
BG_STAT_IN_PROGRESS to be cleared. The background stats thread will
detect this and will eventually quit sooner. */
#define BG_STAT_SHOULD_QUIT (1 << 1)

  /** The state of the background stats thread wrt this table.
  See BG_STAT_NONE, BG_STAT_IN_PROGRESS and BG_STAT_SHOULD_QUIT.
  Writes are covered by dict_sys->mutex. Dirty reads are possible. */
  byte stats_bg_flag;

  /* @} */
#endif /* !UNIV_HOTBACKUP */

  /** AUTOINC related members. @{ */

  /* The actual collection of tables locked during AUTOINC read/write is
  kept in trx_t. In order to quickly determine whether a transaction has
  locked the AUTOINC lock we keep a pointer to the transaction here in
  the 'autoinc_trx' member. This is to avoid acquiring the
  lock_sys_t::mutex and scanning the vector in trx_t.
  When an AUTOINC lock has to wait, the corresponding lock instance is
  created on the trx lock heap rather than use the pre-allocated instance
  in autoinc_lock below. */

  /** A buffer for an AUTOINC lock for this table. We allocate the
  memory here so that individual transactions can get it and release it
  without a need to allocate space from the lock heap of the trx:
  otherwise the lock heap would grow rapidly if we do a large insert
  from a select. */
#ifndef UNIV_HOTBACKUP
  lock_t *autoinc_lock;

  /** Creation state of autoinc_mutex member */
  volatile os_once::state_t autoinc_mutex_created;
#endif /* !UNIV_HOTBACKUP */

  /** Mutex protecting the autoincrement counter. */
  ib_mutex_t *autoinc_mutex;

  /** Autoinc counter value to give to the next inserted row. */
  ib_uint64_t autoinc;

  /** Mutex protecting the persisted autoincrement counter. */
  ib_mutex_t *autoinc_persisted_mutex;

  /** Autoinc counter value that has been persisted in redo logs or
  DDTableBuffer. It's mainly used when we want to write counter back
  to DDTableBuffer.
  This is different from the 'autoinc' above, which could be bigger
  than this one, because 'autoinc' will get updated right after
  some counters are allocated, but we will write the counter to redo
  logs and update this counter later. Once all allocated counters
  have been written to redo logs, 'autoinc' should be exact the next
  counter of this persisted one.
  We want this counter because when we need to write the counter back
  to DDTableBuffer, we had better keep it consistency with the counter
  that has been written to redo logs. Besides, we can't read the 'autoinc'
  directly easily, because the autoinc_lock is required and there could
  be a deadlock.
  This variable is protected by autoinc_persisted_mutex. */
  ib_uint64_t autoinc_persisted;

  /** The position of autoinc counter field in clustered index. This would
  be set when CREATE/ALTER/OPEN TABLE and IMPORT TABLESPACE, and used in
  modifications to clustered index, such as INSERT/UPDATE. There should
  be no conflict to access it, so no protection is needed. */
  ulint autoinc_field_no;

  /** The transaction that currently holds the the AUTOINC lock on this
  table. Protected by lock_sys->mutex. */
  const trx_t *autoinc_trx;

  /* @} */

#ifndef UNIV_HOTBACKUP
  /** FTS specific state variables. */
  fts_t *fts;
#endif /* !UNIV_HOTBACKUP */

  /** Quiescing states, protected by the dict_index_t::lock. ie. we can
  only change the state if we acquire all the latches (dict_index_t::lock)
  in X mode of this table's indexes. */
  ib_quiesce_t quiesce;

  /** Count of the number of record locks on this table. We use this to
  determine whether we can evict the table from the dictionary cache.
  It is protected by lock_sys->mutex. */
  ulint n_rec_locks;

#ifndef UNIV_DEBUG
 private:
#endif
  /** Count of how many handles are opened to this table. Dropping of the
  table is NOT allowed until this count gets to zero. MySQL does NOT
  itself check the number of open handles at DROP. */
  std::atomic<uint64_t> n_ref_count;

 public:
#ifndef UNIV_HOTBACKUP
  /** List of locks on the table. Protected by lock_sys->mutex. */
  table_lock_list_t locks;
  /** count_by_mode[M] = number of locks in this->locks with
  lock->type_mode&LOCK_MODE_MASK == M.
  Used to quickly verify that there are no LOCK_S or LOCK_X, which are the only
  modes incompatible with LOCK_IS and LOCK_IX, to avoid costly iteration over
  this->locks when adding LOCK_IS or LOCK_IX.
  We use count_by_mode[LOCK_AUTO_INC] to track the number of granted and pending
  autoinc locks on this table. This value is set after acquiring the
  lock_sys_t::mutex but we peek the contents to determine whether other
  transactions have acquired the AUTOINC lock or not. Of course only one
  transaction can be granted the lock but there can be multiple
  waiters.
  Protected by lock_sys->mutex. */
  ulong count_by_mode[LOCK_NUM];
#endif /* !UNIV_HOTBACKUP */

  /** Timestamp of the last modification of this table. */
  time_t update_time;

  /** row-id counter for use by intrinsic table for getting row-id.
  Given intrinsic table semantics, row-id can be locally maintained
  instead of getting it from central generator which involves mutex
  locking. */
  ib_uint64_t sess_row_id;

  /** trx_id counter for use by intrinsic table for getting trx-id.
  Intrinsic table are not shared so don't need a central trx-id
  but just need a increased counter to track consistent view while
  proceeding SELECT as part of UPDATE. */
  ib_uint64_t sess_trx_id;

#ifdef UNIV_DEBUG
/** Value of 'magic_n'. */
#define DICT_TABLE_MAGIC_N 76333786

  /** Magic number. */
  ulint magic_n;
#endif /* UNIV_DEBUG */
  /** mysql_row_templ_t for base columns used for compute the virtual
  columns */
  dict_vcol_templ_t *vc_templ;

  /** encryption key, it's only for export/import */
  byte *encryption_key;

  /** encryption iv, it's only for export/import */
  byte *encryption_iv;

  /** remove the dict_table_t from cache after DDL operation */
  bool discard_after_ddl;

  /** refresh/reload FK info */
  bool refresh_fk;

#ifndef UNIV_HOTBACKUP
  /** multiple cursors can be active on this temporary table */
  temp_prebuilt_vec *temp_prebuilt;
#endif /* !UNIV_HOTBACKUP */

  /** TRUE only for dictionary tables like mysql/tables,
  mysql/columns, mysql/tablespaces, etc. This flag is used
  to do non-locking reads on DD tables. */
  bool is_dd_table;

  /** true if this table is explicitly put to non-LRU list
  during table creation */
  bool explicitly_non_lru;

  /** @return the clustered index */
  const dict_index_t *first_index() const {
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);
    const dict_index_t *first = UT_LIST_GET_FIRST(indexes);
    return (first);
  }
  /** @return the clustered index */
  dict_index_t *first_index() {
    return (const_cast<dict_index_t *>(
        const_cast<const dict_table_t *>(this)->first_index()));
  }

  /** @return if there was any instantly added column.
  This will be true after one or more instant ADD COLUMN, however,
  it would become false after ALTER TABLE which rebuilds or copies
  the old table.
  If this is true, all instantly added columns should have default
  values, and records in the table may have REC_INFO_INSTANT_FLAG set. */
  bool has_instant_cols() const {
    ut_ad(n_instant_cols <= n_cols);

    return (n_instant_cols < n_cols);
  }

  /** Set the number of columns when the first instant ADD COLUMN happens.
  @param[in]	instant_cols	number of fields when first instant
                                  ADD COLUMN happens, without system
                                  columns */
  void set_instant_cols(uint16_t instant_cols) {
    n_instant_cols = static_cast<unsigned>(instant_cols) + get_n_sys_cols();
  }

  /** Get the number of user columns when the first instant ADD COLUMN
  happens.
  @return	the number of user columns as described above */
  uint16_t get_instant_cols() const {
    return static_cast<uint16_t>(n_instant_cols - get_n_sys_cols());
  }

  /** Check whether the table is corrupted.
  @return true if the table is corrupted, otherwise false */
  bool is_corrupted() const {
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);

    const dict_index_t *index = first_index();

    /* It is possible that this table is only half created, in which case
    the clustered index may be NULL.  If the clustered index is corrupted,
    the table is corrupt.  We do not consider the table corrupt if only
    a secondary index is corrupt. */
    ut_ad(index == NULL || index->is_clustered());

    return (index != NULL && index->type & DICT_CORRUPT);
  }

  /** Returns a column's name.
  @param[in] col_nr	column number
  @return column name. NOTE: not guaranteed to stay valid if table is
  modified in any way (columns added, etc.). */
  const char *get_col_name(ulint col_nr) const {
    ut_ad(col_nr < n_def);
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);

    const char *s = col_names;
    if (s) {
      for (ulint i = 0; i < col_nr; i++) {
        s += strlen(s) + 1;
      }
    }

    return (s);
  }

  /**Gets the nth column of a table.
  @param[in] pos	position of column
  @return pointer to column object */
  dict_col_t *get_col(ulint pos) const {
    ut_ad(pos < n_def);
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);

    return (cols + pos);
  }

  /** Gets the number of user-defined non-virtual columns in a table
  in the dictionary cache.
  @return number of user-defined (e.g., not ROW_ID) non-virtual columns
  of a table */
  uint16_t get_n_user_cols() const {
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);

    return (static_cast<uint16_t>(n_cols) - get_n_sys_cols());
  }

  /** Gets the number of system columns in a table.
  For intrinsic table on ROW_ID column is added for all other
  tables TRX_ID and ROLL_PTR are all also appeneded.
  @return number of system (e.g., ROW_ID) columns of a table */
  uint16_t get_n_sys_cols() const {
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);

    return (is_intrinsic() ? DATA_ITT_N_SYS_COLS : DATA_N_SYS_COLS);
  }

  /** Gets the number of all non-virtual columns (also system) in a table
  in the dictionary cache.
  @return number of non-virtual columns of a table */
  ulint get_n_cols() const {
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);

    return (n_cols);
  }

  /** Gets the given system column of a table.
  @param[in] sys DATA_ROW_ID, ...
  @return pointer to column object */
  dict_col_t *get_sys_col(ulint sys) const {
    dict_col_t *col;

    ut_ad(sys < get_n_sys_cols());
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);

    col = get_col(n_cols - get_n_sys_cols() + sys);
    ut_ad(col->mtype == DATA_SYS);
    ut_ad(col->prtype == (sys | DATA_NOT_NULL));

    return (col);
  }

  /** Determine if this is a temporary table. */
  bool is_temporary() const {
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);
    return (flags2 & DICT_TF2_TEMPORARY);
  }

  /** Determine if this is a FTS AUX table. */
  bool is_fts_aux() const {
    ut_ad(magic_n == DICT_TABLE_MAGIC_N);
    return (flags2 & DICT_TF2_AUX);
  }

  /** Determine whether the table is intrinsic.
  An intrinsic table is a special kind of temporary table that
  is invisible to the end user. It can be created internally by InnoDB,
  the MySQL server layer or other modules connected to InnoDB in order
  to gather and use data as part of a larger task. Since access to it
  must be as fast as possible, it does not need UNDO semantics, system
  fields DB_TRX_ID & DB_ROLL_PTR, doublewrite, checksum, insert buffer,
  use of the shared data dictionary, locking, or even a transaction.
  In short, these are not ACID tables at all, just temporary data stored
  and manipulated during a larger process.*/
  bool is_intrinsic() const {
    if (flags2 & DICT_TF2_INTRINSIC) {
      ut_ad(is_temporary());
      return (true);
    }

    return (false);
  }

  /* GAP locks are skipped for DD tables and SDI tables
  @return true if table is DD table or SDI table, else false */
  bool skip_gap_locks() const;

  /** Determine if the table can support instant ADD COLUMN */
  bool support_instant_add() const;
};
