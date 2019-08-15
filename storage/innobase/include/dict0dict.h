/*****************************************************************************

Copyright (c) 1996, 2019, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2012, Facebook Inc.

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

/** @file include/dict0dict.h
 Data dictionary system

 Created 1/8/1996 Heikki Tuuri
 *******************************************************/

#ifndef dict0dict_h
#define dict0dict_h

#include <innodb/univ/univ.h>

#include <innodb/data_types/flags.h>
#include <innodb/dict_mem/dict_col_name_is_reserved.h>
#include <innodb/dict_mem/dict_foreign_error_report_low.h>
#include <innodb/dict_mem/dict_foreign_find.h>
#include <innodb/dict_mem/dict_foreign_free.h>
#include <innodb/dict_mem/dict_foreign_print.h>
#include <innodb/dict_mem/dict_foreign_remove_from_cache.h>
#include <innodb/dict_mem/dict_get_db_name_len.h>
#include <innodb/dict_mem/dict_index_calc_min_rec_len.h>
#include <innodb/dict_mem/dict_index_contains_col_or_prefix.h>
#include <innodb/dict_mem/dict_index_copy_types.h>
#include <innodb/dict_mem/dict_index_find.h>
#include <innodb/dict_mem/dict_index_get_nth_field_pos.h>
#include <innodb/dict_mem/dict_index_remove_from_v_col_list.h>
#include <innodb/dict_mem/dict_move_to_mru.h>
#include <innodb/dict_mem/dict_mutex_enter_for_mysql.h>
#include <innodb/dict_mem/dict_mutex_exit_for_mysql.h>
#include <innodb/dict_mem/dict_remove_db_name.h>
#include <innodb/dict_mem/dict_sys.h>
#include <innodb/dict_mem/dict_sys_t.h>
#include <innodb/dict_mem/dict_table_add_system_columns.h>
#include <innodb/dict_mem/dict_table_add_to_cache.h>
#include <innodb/dict_mem/dict_table_autoinc_initialize.h>
#include <innodb/dict_mem/dict_table_autoinc_lock.h>
#include <innodb/dict_mem/dict_table_autoinc_read.h>
#include <innodb/dict_mem/dict_table_autoinc_set_col_pos.h>
#include <innodb/dict_mem/dict_table_autoinc_unlock.h>
#include <innodb/dict_mem/dict_table_autoinc_update_if_greater.h>
#include <innodb/dict_mem/dict_table_change_id_in_cache.h>
#include <innodb/dict_mem/dict_table_col_in_clustered_key.h>
#include <innodb/dict_mem/dict_table_copy_types.h>
#include <innodb/dict_mem/dict_table_copy_v_types.h>
#include <innodb/dict_mem/dict_table_extent_size.h>
#include <innodb/dict_mem/dict_table_find_index_on_id.h>
#include <innodb/dict_mem/dict_table_get_all_fts_indexes.h>
#include <innodb/dict_mem/dict_table_get_highest_foreign_id.h>
#include <innodb/dict_mem/dict_table_get_index_on_name.h>
#include <innodb/dict_mem/dict_table_get_n_tot_u_cols.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_table_get_v_col_name.h>
#include <innodb/dict_mem/dict_table_has_column.h>
#include <innodb/dict_mem/dict_table_move_from_lru_to_non_lru.h>
#include <innodb/dict_mem/dict_table_move_from_non_lru_to_lru.h>
#include <innodb/dict_mem/dict_table_mysql_pos_to_innodb.h>
#include <innodb/dict_mem/dict_table_next_uncorrupted_index.h>
#include <innodb/dict_mem/dict_table_op_t.h>
#include <innodb/dict_mem/dict_table_set_big_rows.h>
#include <innodb/dict_mem/dict_table_skip_corrupt_index.h>
#include <innodb/dict_mem/dict_table_stats_latch_alloc.h>
#include <innodb/dict_mem/dict_table_stats_latch_create.h>
#include <innodb/dict_mem/dict_table_stats_latch_destroy.h>
#include <innodb/dict_mem/dict_table_stats_latch_free.h>
#include <innodb/dict_mem/dict_table_stats_lock.h>
#include <innodb/dict_mem/dict_table_stats_unlock.h>
#include <innodb/dict_mem/dict_tables_have_same_db.h>
#include <innodb/dict_mem/rec_format_t.h>
#include <innodb/dict_types/DictSysMutex.h>
#include <innodb/dict_types/dict_err_ignore_t.h>
#include <innodb/hash/HASH_GET_FIRST.h>
#include <innodb/hash/HASH_GET_NEXT.h>
#include <innodb/hash/hash_table_t.h>
#include <innodb/random/random.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/dict_sys_t_is_reserved.h>
#include <innodb/dict_mem/dict_create_v_col_pos.h>
#include <innodb/dict_mem/dict_get_v_col_mysql_pos.h>
#include <innodb/dict_mem/dict_get_v_col_pos.h>

#include "buf0flu.h"
#include <set>
#include <deque>
#include "btr0types.h"
#include "dict/dict.h"
#include "dict0mem.h"
#include "fsp0fsp.h"
#include "row0types.h"
#include "sql/dd/object_id.h"


struct dict_sys_t;



#ifndef UNIV_HOTBACKUP
/** Open a table from its database and table name, this is currently used by
 foreign constraint parser to get the referenced table.
 @return complete table name with database and table name, allocated from
 heap memory passed in */
char *dict_get_referenced_table(
    const char *name,          /*!< in: foreign key table name */
    const char *database_name, /*!< in: table db name */
    ulint database_name_len,   /*!< in: db name length */
    const char *table_name,    /*!< in: table name */
    ulint table_name_len,      /*!< in: table name length */
    dict_table_t **table,      /*!< out: table object or NULL */
    mem_heap_t *heap);         /*!< in: heap memory */
/** Frees a foreign key struct. */


#endif                    /* !UNIV_HOTBACKUP */



/** Decrements the count of open handles to a table. */
void dict_table_close(dict_table_t *table, /*!< in/out: table */
                      ibool dict_locked, /*!< in: TRUE=data dictionary locked */
                      ibool try_drop);   /*!< in: TRUE=try to drop any orphan
                                         indexes after an aborted online
                                         index creation */
/** Closes the only open handle to a table and drops a table while assuring
 that dict_sys->mutex is held the whole time.  This assures that the table
 is not evicted after the close when the count of open handles goes to zero.
 Because dict_sys->mutex is held, we do not need to call
 dict_table_prevent_eviction().  */
void dict_table_close_and_drop(
    trx_t *trx,           /*!< in: data dictionary transaction */
    dict_table_t *table); /*!< in/out: table */
/** Inits the data dictionary module. */
void dict_init(void);

/** Inits the structure for persisting dynamic metadata */
void dict_persist_init(void);

/** Clear the structure */
void dict_persist_close(void);

#ifndef UNIV_HOTBACKUP
/** Write back the dirty persistent dynamic metadata of the table
to DDTableBuffer.
@param[in,out]	table	table object */
void dict_table_persist_to_dd_table_buffer(dict_table_t *table);

/** Read persistent dynamic metadata stored in a buffer
@param[in]	buffer		buffer to read
@param[in]	size		size of data in buffer
@param[in]	metadata	where we store the metadata from buffer */
void dict_table_read_dynamic_metadata(const byte *buffer, ulint size,
                                      PersistentTableMetadata *metadata);

#endif /* !UNIV_HOTBACKUP */





#ifndef UNIV_HOTBACKUP

/** Write redo logs for autoinc counter that is to be inserted, or to
update some existing smaller one to bigger.
@param[in,out]	table	InnoDB table object
@param[in]	value	AUTOINC counter to log
@param[in,out]	mtr	mini-transaction */
void dict_table_autoinc_log(dict_table_t *table, uint64_t value, mtr_t *mtr);

#endif /* !UNIV_HOTBACKUP */




#ifndef UNIV_HOTBACKUP



/** Removes a table object from the dictionary cache. */
void dict_table_remove_from_cache(dict_table_t *table); /*!< in, own: table */

/** Try to invalidate an entry from the dict cache, for a partitioned table,
if any table found.
@param[in]	name	Table name */
void dict_partitioned_table_remove_from_cache(const char *name);

#ifdef UNIV_DEBUG
/** Removes a table object from the dictionary cache, for debug purpose
@param[in,out]	table		table object
@param[in]	lru_evict	true if table being evicted to make room
                                in the table LRU list */
void dict_table_remove_from_cache_debug(dict_table_t *table, bool lru_evict);
#endif /* UNIV_DEBUG */

/** Renames a table object.
 @return true if success */
dberr_t dict_table_rename_in_cache(dict_table_t *table,  /*!< in/out: table */
                                   const char *new_name, /*!< in: new name */
                                   ibool rename_also_foreigns)
    /*!< in: in ALTER TABLE we want
    to preserve the original table name
    in constraints which reference it */
    MY_ATTRIBUTE((warn_unused_result));

/** Removes an index from the dictionary cache.
@param[in,out]	table	table whose index to remove
@param[in,out]	index	index to remove, this object is destroyed and must not
be accessed by the caller afterwards */
void dict_index_remove_from_cache(dict_table_t *table, dict_index_t *index);



/** Adds a foreign key constraint object to the dictionary cache. May free
 the object if there already is an object with the same identifier in.
 At least one of foreign table or referenced table must already be in
 the dictionary cache!
 @return DB_SUCCESS or error code */
dberr_t dict_foreign_add_to_cache(dict_foreign_t *foreign,
                                  /*!< in, own: foreign key constraint */
                                  const char **col_names,
                                  /*!< in: column names, or NULL to use
                                  foreign->foreign_table->col_names */
                                  bool check_charsets,
                                  /*!< in: whether to check charset
                                  compatibility */
                                  bool can_free_fk,
                                  /*!< in: whether free existing FK */
                                  dict_err_ignore_t ignore_err)
    /*!< in: error to be ignored */
    MY_ATTRIBUTE((warn_unused_result));
/** Checks if a table is referenced by foreign keys.
 @return true if table is referenced by a foreign key */
ibool dict_table_is_referenced_by_foreign_key(
    const dict_table_t *table) /*!< in: InnoDB table */
    MY_ATTRIBUTE((warn_unused_result));
/** Replace the index passed in with another equivalent index in the
 foreign key lists of the table.
 @return whether all replacements were found */
bool dict_foreign_replace_index(
    dict_table_t *table, /*!< in/out: table */
    const char **col_names,
    /*!< in: column names, or NULL
    to use table->col_names */
    const dict_index_t *index) /*!< in: index to be replaced */
    MY_ATTRIBUTE((warn_unused_result));
/** Scans a table create SQL string and adds to the data dictionary
the foreign key constraints declared in the string. This function
should be called after the indexes for a table have been created.
Each foreign key constraint must be accompanied with indexes in
bot participating tables. The indexes are allowed to contain more
fields than mentioned in the constraint.

@param[in]	trx		transaction
@param[in]	sql_string	table create statement where
                                foreign keys are declared like:
                                FOREIGN KEY (a, b) REFERENCES table2(c, d),
                                table2 can be written also with the database
                                name before it: test.table2; the default
                                database id the database of parameter name
@param[in]	sql_length	length of sql_string
@param[in]	name		table full name in normalized form
@param[in]	reject_fks	if TRUE, fail with error code
                                DB_CANNOT_ADD_CONSTRAINT if any
                                foreign keys are found.
@return error code or DB_SUCCESS */
dberr_t dict_create_foreign_constraints(trx_t *trx, const char *sql_string,
                                        size_t sql_length, const char *name,
                                        ibool reject_fks)
    MY_ATTRIBUTE((warn_unused_result));
/** Parses the CONSTRAINT id's to be dropped in an ALTER TABLE statement.
 @return DB_SUCCESS or DB_CANNOT_DROP_CONSTRAINT if syntax error or the
 constraint id does not match */
dberr_t dict_foreign_parse_drop_constraints(
    mem_heap_t *heap,                  /*!< in: heap from which we can
                                       allocate memory */
    trx_t *trx,                        /*!< in: transaction */
    dict_table_t *table,               /*!< in: table */
    ulint *n,                          /*!< out: number of constraints
                                       to drop */
    const char ***constraints_to_drop) /*!< out: id's of the
                                       constraints to drop */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* !UNIV_HOTBACKUP */
/** Returns a table object and increments its open handle count.
 NOTE! This is a high-level function to be used mainly from outside the
 'dict' directory. Inside this directory dict_table_get_low
 is usually the appropriate function.
 @param[in] table_name Table name
 @param[in] dict_locked TRUE=data dictionary locked
 @param[in] try_drop TRUE=try to drop any orphan indexes after
                                 an aborted online index creation
 @param[in] ignore_err error to be ignored when loading the table
 @return table, NULL if does not exist */
dict_table_t *dict_table_open_on_name(const char *table_name, ibool dict_locked,
                                      ibool try_drop,
                                      dict_err_ignore_t ignore_err)
    MY_ATTRIBUTE((warn_unused_result));

/** Tries to find an index whose first fields are the columns in the array,
 in the same order and is not marked for deletion and is not the same
 as types_idx.
 @return matching index, NULL if not found */
dict_index_t *dict_foreign_find_index(
    const dict_table_t *table, /*!< in: table */
    const char **col_names,
    /*!< in: column names, or NULL
    to use table->col_names */
    const char **columns, /*!< in: array of column names */
    ulint n_cols,         /*!< in: number of columns */
    const dict_index_t *types_idx,
    /*!< in: NULL or an index
    whose types the column types
    must match */
    bool check_charsets,
    /*!< in: whether to check
    charsets.  only has an effect
    if types_idx != NULL */
    ulint check_null)
    /*!< in: nonzero if none of
    the columns must be declared
    NOT NULL */
    MY_ATTRIBUTE((warn_unused_result));




/** Outputs info on foreign keys of a table. */
void dict_print_info_on_foreign_keys(
    ibool create_table_format, /*!< in: if TRUE then print in
                  a format suitable to be inserted into
                  a CREATE TABLE, otherwise in the format
                  of SHOW TABLE STATUS */
    FILE *file,                /*!< in: file where to print */
    trx_t *trx,                /*!< in: transaction */
    dict_table_t *table);      /*!< in: table */
/** Outputs info on a foreign key of a table in a format suitable for
 CREATE TABLE. */
void dict_print_info_on_foreign_key_in_create_format(
    FILE *file,              /*!< in: file where to print */
    trx_t *trx,              /*!< in: transaction */
    dict_foreign_t *foreign, /*!< in: foreign key constraint */
    ibool add_newline);      /*!< in: whether to add a newline */
/** Tries to find an index whose first fields are the columns in the array,
 in the same order and is not marked for deletion and is not the same
 as types_idx.
 @return matching index, NULL if not found */
bool dict_foreign_qualify_index(
    const dict_table_t *table, /*!< in: table */
    const char **col_names,
    /*!< in: column names, or NULL
    to use table->col_names */
    const char **columns,      /*!< in: array of column names */
    ulint n_cols,              /*!< in: number of columns */
    const dict_index_t *index, /*!< in: index to check */
    const dict_index_t *types_idx,
    /*!< in: NULL or an index
    whose types the column types
    must match */
    bool check_charsets,
    /*!< in: whether to check
    charsets.  only has an effect
    if types_idx != NULL */
    ulint check_null)
    /*!< in: nonzero if none of
    the columns must be declared
    NOT NULL */
    MY_ATTRIBUTE((warn_unused_result));



/** Get nth virtual column
@param[in]	table	target table
@param[in]	col_nr	column number in MySQL Table definition
@return dict_v_col_t ptr */
dict_v_col_t *dict_table_get_nth_v_col_mysql(const dict_table_t *table,
                                             ulint col_nr);



#ifndef UNIV_HOTBACKUP




/** Convert a 32 bit integer table flags to the 32 bit FSP Flags.
Fsp Flags are written into the tablespace header at the offset
FSP_SPACE_FLAGS and are also stored in the fil_space_t::flags field.
The following chart shows the translation of the low order bit.
Other bits are the same.
========================= Low order bit ==========================
                    | REDUNDANT | COMPACT | COMPRESSED | DYNAMIC
dict_table_t::flags |     0     |    1    |     1      |    1
fil_space_t::flags  |     0     |    0    |     1      |    1
==================================================================
@param[in]	table_flags	dict_table_t::flags
@return tablespace flags (fil_space_t::flags) */
uint32_t dict_tf_to_fsp_flags(uint32_t table_flags) MY_ATTRIBUTE((const));

/** Extract the page size from table flags.
@param[in]	flags	flags
@return compressed page size, or 0 if not compressed */
UNIV_INLINE
const page_size_t dict_tf_get_page_size(uint32_t flags) MY_ATTRIBUTE((const));
#endif /* !UNIV_HOTBACKUP */




#ifndef UNIV_HOTBACKUP
/********************************************************************
Wait until all the background threads of the given table have exited, i.e.,
bg_threads == 0. Note: bg_threads_mutex must be reserved when
calling this. */
void dict_table_wait_for_bg_threads_to_exit(
    dict_table_t *table, /* in: table */
    ulint delay);        /* in: time in microseconds to wait between
                         checks of bg_threads. */

/** Make room in the table cache by evicting an unused table. The unused table
 should not be part of FK relationship and currently not used in any user
 transaction. There is no guarantee that it will remove a table.
 @return number of tables evicted. */
ulint dict_make_room_in_cache(
    ulint max_tables, /*!< in: max tables allowed in cache */
    ulint pct_check); /*!< in: max percent to check */



/** Adds an index to the dictionary cache.
@param[in]	table	table on which the index is
@param[in]	index	index; NOTE! The index memory
                        object is freed in this function!
@param[in]	page_no	root page number of the index
@param[in]	strict	TRUE=refuse to create the index
                        if records could be too big to fit in
                        an B-tree page
@return DB_SUCCESS, DB_TOO_BIG_RECORD, or DB_CORRUPTION */
dberr_t dict_index_add_to_cache(
    dict_table_t *table, /*!< in: table on which the index is */
    dict_index_t *index, /*!< in, own: index; NOTE! The index memory
                         object is freed in this function! */
    page_no_t page_no,   /*!< in: root page number of the index */
    ibool strict)        /*!< in: TRUE=refuse to create the index
                         if records could be too big to fit in
                         an B-tree page */
    MY_ATTRIBUTE((warn_unused_result));



/** Adds an index to the dictionary cache, with possible indexing newly
added column.
@param[in]	table	table on which the index is
@param[in]	index	index; NOTE! The index memory
                        object is freed in this function!
@param[in]	add_v	new virtual column that being added along with
                        an add index call
@param[in]	page_no	root page number of the index
@param[in]	strict	TRUE=refuse to create the index
                        if records could be too big to fit in
                        an B-tree page
@return DB_SUCCESS, DB_TOO_BIG_RECORD, or DB_CORRUPTION */
dberr_t dict_index_add_to_cache_w_vcol(dict_table_t *table, dict_index_t *index,
                                       const dict_add_v_col_t *add_v,
                                       page_no_t page_no, ibool strict)
    MY_ATTRIBUTE((warn_unused_result));
#endif /* !UNIV_HOTBACKUP */












#ifdef UNIV_DEBUG
/** Checks that a tuple has n_fields_cmp value in a sensible range, so that
 no comparison can occur with the page number field in a node pointer.
 @return true if ok */
ibool dict_index_check_search_tuple(
    const dict_index_t *index, /*!< in: index tree */
    const dtuple_t *tuple)     /*!< in: tuple used in a search */
    MY_ATTRIBUTE((warn_unused_result));
/** Whether and when to allow temporary index names */
enum check_name {
  /** Require all indexes to be complete. */
  CHECK_ALL_COMPLETE,
  /** Allow aborted online index creation. */
  CHECK_ABORTED_OK,
  /** Allow partial indexes to exist. */
  CHECK_PARTIAL_OK
};
/** Check for duplicate index entries in a table [using the index name] */
void dict_table_check_for_dup_indexes(
    const dict_table_t *table, /*!< in: Check for dup indexes
                               in this table */
    enum check_name check);    /*!< in: whether and when to allow
                               temporary index names */
/** Check if a table is a temporary table with compressed row format,
we should always expect false.
@param[in]	table	table
@return true if it's a compressed temporary table, false otherwise */
inline bool dict_table_is_compressed_temporary(const dict_table_t *table);
#endif /* UNIV_DEBUG */


/** Builds a node pointer out of a physical record and a page number.
 @return own: node pointer */
dtuple_t *dict_index_build_node_ptr(
    const dict_index_t *index, /*!< in: index */
    const rec_t *rec,          /*!< in: record for which to build node
                               pointer */
    page_no_t page_no,         /*!< in: page number to put in node
                               pointer */
    mem_heap_t *heap,          /*!< in: memory heap where pointer
                               created */
    ulint level)               /*!< in: level of rec in tree:
                               0 means leaf level */
    MY_ATTRIBUTE((warn_unused_result));
/** Copies an initial segment of a physical record, long enough to specify an
 index entry uniquely.
 @return pointer to the prefix record */
rec_t *dict_index_copy_rec_order_prefix(
    const dict_index_t *index, /*!< in: index */
    const rec_t *rec,          /*!< in: record for which to
                               copy prefix */
    ulint *n_fields,           /*!< out: number of fields copied */
    byte **buf,                /*!< in/out: memory buffer for the
                               copied prefix, or NULL */
    size_t *buf_size)          /*!< in/out: buffer size */
    MY_ATTRIBUTE((warn_unused_result));
/** Builds a typed data tuple out of a physical record.
 @return own: data tuple */
dtuple_t *dict_index_build_data_tuple(
    dict_index_t *index, /*!< in: index */
    rec_t *rec,          /*!< in: record for which to build data tuple */
    ulint n_fields,      /*!< in: number of data fields */
    mem_heap_t *heap)    /*!< in: memory heap where tuple created */
    MY_ATTRIBUTE((warn_unused_result));





#ifndef UNIV_HOTBACKUP












/* Buffers for storing detailed information about the latest foreign key
and unique key errors */
extern FILE *dict_foreign_err_file;
extern ib_mutex_t dict_foreign_err_mutex; /* mutex protecting the
                                          foreign key error messages */
#endif                                    /* !UNIV_HOTBACKUP */


#ifndef UNIV_HOTBACKUP
/** the data dictionary rw-latch protecting dict_sys */
extern rw_lock_t *dict_operation_lock;

/** Forward declaration */
class DDTableBuffer;
#endif /* !UNIV_HOTBACKUP */
struct dict_persist_t;

/** the dictionary persisting structure */
extern dict_persist_t *dict_persist;

/** Structure for persisting dynamic metadata of data dictionary */
struct dict_persist_t {
  /** Mutex to protect data in this structure, also the
  dict_table_t::dirty_status and
  dict_table_t::in_dirty_dict_tables_list
  This mutex should be low-level one so that it can be used widely
  when necessary, so its level had to be above SYNC_LOG. However,
  after this mutex, persister may have to access B-tree and require
  tree latch, the latch level of this mutex then has to be right
  before the SYNC_INDEX_TREE. */
  ib_mutex_t mutex;

  /** List of tables whose dirty_status are marked as METADATA_DIRTY,
  or METADATA_BUFFERED. It's protected by the mutex */
  UT_LIST_BASE_NODE_T(dict_table_t)
  dirty_dict_tables;

  /** Number of the tables which are of status METADATA_DIRTY.
  It's protected by the mutex */
  std::atomic<uint32_t> num_dirty_tables;

#ifndef UNIV_HOTBACKUP
  /** DDTableBuffer table for persistent dynamic metadata */
  DDTableBuffer *table_buffer;
#endif /* !UNIV_HOTBACKUP */

  /** Collection of instances to persist dynamic metadata */
  Persisters *persisters;
};

#ifndef UNIV_HOTBACKUP
/** dummy index for ROW_FORMAT=REDUNDANT supremum and infimum records */
extern dict_index_t *dict_ind_redundant;

/** Inits dict_ind_redundant. */
void dict_ind_init(void);

/** Converts a database and table name from filesystem encoding (e.g.
"@code d@i1b/a@q1b@1Kc @endcode", same format as used in  dict_table_t::name)
in two strings in UTF8 encoding (e.g. dцb and aюbØc). The output buffers must
be at least MAX_DB_UTF8_LEN and MAX_TABLE_UTF8_LEN bytes.
@param[in]	db_and_table	database and table names,
                                e.g. "@code d@i1b/a@q1b@1Kc @endcode"
@param[out]	db_utf8		database name, e.g. dцb
@param[in]	db_utf8_size	dbname_utf8 size
@param[out]	table_utf8	table name, e.g. aюbØc
@param[in]	table_utf8_size	table_utf8 size */
void dict_fs2utf8(const char *db_and_table, char *db_utf8, size_t db_utf8_size,
                  char *table_utf8, size_t table_utf8_size);

/** Resize the hash tables besed on the current buffer pool size. */
void dict_resize();

/** Closes the data dictionary module. */
void dict_close(void);

/** Wrapper for the mysql.innodb_dynamic_metadata used to buffer the persistent
dynamic metadata.
This should be a table with only clustered index, no delete-marked records,
no locking, no undo logging, no purge, no adaptive hash index.
We should always use low level btr functions to access and modify the table.
Accessing this table should be protected by dict_sys->mutex */
class DDTableBuffer {
 public:
  /** Default constructor */
  DDTableBuffer();

  /** Destructor */
  ~DDTableBuffer();

  /** Replace the dynamic metadata for a specific table
  @param[in]	id		table id
  @param[in]	version		table dynamic metadata version
  @param[in]	metadata	the metadata we want to replace
  @param[in]	len		the metadata length
  @return DB_SUCCESS or error code */
  dberr_t replace(table_id_t id, uint64_t version, const byte *metadata,
                  size_t len);

  /** Remove the whole row for a specific table
  @param[in]	id	table id
  @return DB_SUCCESS or error code */
  dberr_t remove(table_id_t id);

  /** Truncate the table. We can call it after all the dynamic
  metadata has been written back to DD table */
  void truncate(void);

  /** Get the buffered metadata for a specific table, the caller
  has to delete the returned std::string object by UT_DELETE
  @param[in]	id	table id
  @param[out]	version	table dynamic metadata version
  @return the metadata saved in a string object, if nothing, the
  string would be of length 0 */
  std::string *get(table_id_t id, uint64 *version);

 private:
  /** Initialize m_index, the in-memory clustered index of the table
  and two tuples used in this class */
  void init();

  /** Open the mysql.innodb_dynamic_metadata when DD is not fully up */
  void open();

  /** Create the search and replace tuples */
  void create_tuples();

  /** Initialize the id field of tuple
  @param[out]	tuple	the tuple to be initialized
  @param[in]	id	table id */
  void init_tuple_with_id(dtuple_t *tuple, table_id_t id);

  /** Free the things initialized in init() */
  void close();

  /** Prepare for a update on METADATA field
  @param[in]	entry	clustered index entry to replace rec
  @param[in]	rec	clustered index record
  @return update vector of differing fields without system columns,
  or NULL if there isn't any different field */
  upd_t *update_set_metadata(const dtuple_t *entry, const rec_t *rec);

 private:
  /** The clustered index of this system table */
  dict_index_t *m_index;

  /** The heap used for dynamic allocations, which should always
  be freed before return */
  mem_heap_t *m_dynamic_heap;

  /** The heap used during replace() operation, which should always
  be freed before return */
  mem_heap_t *m_replace_heap;

  /** The heap used to create the search tuple and replace tuple */
  mem_heap_t *m_heap;

  /** The tuple used to search for specified table, it's protected
  by dict_persist->mutex */
  dtuple_t *m_search_tuple;

  /** The tuple used to replace for specified table, it's protected
  by dict_persist->mutex */
  dtuple_t *m_replace_tuple;

 private:
  /** Column number of mysql.innodb_dynamic_metadata.table_id */
  static constexpr unsigned TABLE_ID_COL_NO = 0;

  /** Column number of mysql.innodb_dynamic_metadata.version */
  static constexpr unsigned VERSION_COL_NO = 1;

  /** Column number of mysql.innodb_dynamic_metadata.metadata */
  static constexpr unsigned METADATA_COL_NO = 2;

  /** Number of user columns */
  static constexpr unsigned N_USER_COLS = METADATA_COL_NO + 1;

  /** Number of columns */
  static constexpr unsigned N_COLS = N_USER_COLS + DATA_N_SYS_COLS;

  /** Clustered index field number of
  mysql.innodb_dynamic_metadata.table_id */
  static constexpr unsigned TABLE_ID_FIELD_NO = TABLE_ID_COL_NO;

  /** Clustered index field number of
  mysql.innodb_dynamic_metadata.version */
  static constexpr unsigned VERSION_FIELD_NO = VERSION_COL_NO + 2;

  /** Clustered index field number of
  mysql.innodb_dynamic_metadata.metadata
  Plusing 2 here skips the DATA_TRX_ID and DATA_ROLL_PTR fields */
  static constexpr unsigned METADATA_FIELD_NO = METADATA_COL_NO + 2;

  /** Number of fields in the clustered index */
  static constexpr unsigned N_FIELDS = METADATA_FIELD_NO + 1;
};

/** Mark the dirty_status of a table as METADATA_DIRTY, and add it to the
dirty_dict_tables list if necessary.
@param[in,out]	table		table */
void dict_table_mark_dirty(dict_table_t *table);
#endif /* !UNIV_HOTBACKUP */

/** Flags an index corrupted in the data dictionary cache only. This
is used to mark a corrupted index when index's own dictionary
is corrupted, and we would force to load such index for repair purpose.
Besides, we have to write a redo log.
We don't want to hold dict_sys->mutex here, so that we can set index as
corrupted in some low-level functions. We would only set the flags from
not corrupted to corrupted when server is running, so it should be safe
to set it directly.
@param[in,out]	index		index, must not be NULL */
void dict_set_corrupted(dict_index_t *index) UNIV_COLD;

#ifndef UNIV_HOTBACKUP
/** Check if there is any latest persistent dynamic metadata recorded
in DDTableBuffer table of the specific table. If so, read the metadata and
update the table object accordingly
@param[in]	table		table object */
void dict_table_load_dynamic_metadata(dict_table_t *table);

/** Check if any table has any dirty persistent data, if so
write dirty persistent data of table to mysql.innodb_dynamic_metadata
accordingly. */
void dict_persist_to_dd_table_buffer();
#endif /* !UNIV_HOTBACKUP */

/** Apply the persistent dynamic metadata read from redo logs or
DDTableBuffer to corresponding table during recovery.
@param[in,out]	table		table
@param[in]	metadata	structure of persistent metadata
@return true if we do apply something to the in-memory table object,
otherwise false */
bool dict_table_apply_dynamic_metadata(dict_table_t *table,
                                       const PersistentTableMetadata *metadata);

#ifndef UNIV_HOTBACKUP
/** Sets merge_threshold in the SYS_INDEXES
@param[in,out]	index		index
@param[in]	merge_threshold	value to set */
void dict_index_set_merge_threshold(dict_index_t *index, ulint merge_threshold);

#ifdef UNIV_DEBUG
/** Sets merge_threshold for all indexes in dictionary cache for debug.
@param[in]	merge_threshold_all	value to set for all indexes */
void dict_set_merge_threshold_all_debug(uint merge_threshold_all);
#endif /* UNIV_DEBUG */

/** Validate the table flags.
@param[in]	flags	Table flags
@return true if valid. */
UNIV_INLINE
bool dict_tf_is_valid(uint32_t flags);

/** Validate both table flags and table flags2 and make sure they
are compatible.
@param[in]	flags	Table flags
@param[in]	flags2	Table flags2
@return true if valid. */
UNIV_INLINE
bool dict_tf2_is_valid(uint32_t flags, uint32_t flags2);

/** Check if the tablespace for the table has been discarded.
 @return true if the tablespace has been discarded. */
UNIV_INLINE
bool dict_table_is_discarded(
    const dict_table_t *table) /*!< in: table to check */
    MY_ATTRIBUTE((warn_unused_result));

/** Check whether the table is DDTableBuffer. See class DDTableBuffer
@param[in]	table	table to check
@return true if this is a DDTableBuffer table. */
UNIV_INLINE
bool dict_table_is_table_buffer(const dict_table_t *table);

/** Check if the table is in a shared tablespace (System or General).
@param[in]	table	table to check
@return true if table is a shared tablespace, false if not. */
UNIV_INLINE
bool dict_table_in_shared_tablespace(const dict_table_t *table)
    MY_ATTRIBUTE((warn_unused_result));

/** Check whether locking is disabled for this table.
Currently this is done for intrinsic table as their visibility is limited
to the connection and the DDTableBuffer as it's protected by
dict_persist->mutex.

@param[in]	table	table to check
@return true if locking is disabled. */
UNIV_INLINE
bool dict_table_is_locking_disabled(const dict_table_t *table)
    MY_ATTRIBUTE((warn_unused_result));

/** Turn-off redo-logging if temporary table.
@param[in]	table	table to check
@param[out]	mtr	mini-transaction */
UNIV_INLINE
void dict_disable_redo_if_temporary(const dict_table_t *table, mtr_t *mtr);

/** Get table session row-id and increment the row-id counter for next use.
@param[in,out]	table	table handler
@return next table local row-id. */
UNIV_INLINE
row_id_t dict_table_get_next_table_sess_row_id(dict_table_t *table);

/** Get table session trx-id and increment the trx-id counter for next use.
@param[in,out]	table	table handler
@return next table local trx-id. */
UNIV_INLINE
trx_id_t dict_table_get_next_table_sess_trx_id(dict_table_t *table);

/** Get current session trx-id.
@param[in]	table	table handler
@return table local trx-id. */
UNIV_INLINE
trx_id_t dict_table_get_curr_table_sess_trx_id(const dict_table_t *table);

/** This function should be called whenever a page is successfully
 compressed. Updates the compression padding information. */
void dict_index_zip_success(
    dict_index_t *index); /*!< in/out: index to be updated. */
/** This function should be called whenever a page compression attempt
 fails. Updates the compression padding information. */
void dict_index_zip_failure(
    dict_index_t *index); /*!< in/out: index to be updated. */
/** Return the optimal page size, for which page will likely compress.
 @return page size beyond which page may not compress*/
ulint dict_index_zip_pad_optimal_page_size(
    dict_index_t *index) /*!< in: index for which page size
                         is requested */
    MY_ATTRIBUTE((warn_unused_result));
/** Convert table flag to row format string.
 @return row format name */
const char *dict_tf_to_row_format_string(
    ulint table_flag); /*!< in: row format setting */
/** Return maximum size of the node pointer record.
 @return maximum size of the record in bytes */
ulint dict_index_node_ptr_max_size(const dict_index_t *index) /*!< in: index */
    MY_ATTRIBUTE((warn_unused_result));

/** Get index by first field of the index.
@param[in]	table		table
@param[in]	col_index	position of column in table
@return index which is having first field matches with the field present in
field_index position of table */
UNIV_INLINE
dict_index_t *dict_table_get_index_on_first_col(dict_table_t *table,
                                                ulint col_index);
#endif /* !UNIV_HOTBACKUP */

/** encode number of columns and number of virtual columns in one
4 bytes value. We could do this because the number of columns in
InnoDB is limited to 1017
@param[in]	n_col	number of non-virtual column
@param[in]	n_v_col	number of virtual column
@return encoded value */
UNIV_INLINE
ulint dict_table_encode_n_col(ulint n_col, ulint n_v_col);

/** Decode number of virtual and non-virtual columns in one 4 bytes value.
@param[in]	encoded	encoded value
@param[in,out]	n_col	number of non-virtual column
@param[in,out]	n_v_col	number of virtual column */
UNIV_INLINE
void dict_table_decode_n_col(uint32_t encoded, uint32_t *n_col,
                             uint32_t *n_v_col);

/** Free the virtual column template
@param[in,out]	vc_templ	virtual column template */
UNIV_INLINE
void dict_free_vc_templ(dict_vcol_templ_t *vc_templ);

/** Returns a virtual column's name according to its original
MySQL table position.
@param[in]	table	target table
@param[in]	col_nr	column number (nth column in the table)
@return column name. */
const char *dict_table_get_v_col_name_mysql(const dict_table_t *table,
                                            ulint col_nr);

/** Check whether the table have virtual index.
@param[in]	table	InnoDB table
@return true if the table have virtual index, false otherwise. */
UNIV_INLINE
bool dict_table_have_virtual_index(dict_table_t *table);

/** Retrieve in-memory index for SDI table.
@param[in]	tablespace_id	innodb tablespace ID
@return dict_index_t structure or NULL*/
dict_index_t *dict_sdi_get_index(space_id_t tablespace_id);

/** Retrieve in-memory table object for SDI table.
@param[in]	tablespace_id	innodb tablespace ID
@param[in]	dict_locked	true if dict_sys mutex is acquired
@param[in]	is_create	true when creating SDI Index
@return dict_table_t structure */
dict_table_t *dict_sdi_get_table(space_id_t tablespace_id, bool dict_locked,
                                 bool is_create);

/** Remove the SDI table from table cache.
@param[in]	space_id	InnoDB tablespace ID
@param[in]	sdi_table	SDI table
@param[in]	dict_locked	true if dict_sys mutex acquired */
void dict_sdi_remove_from_cache(space_id_t space_id, dict_table_t *sdi_table,
                                bool dict_locked);

/** Check if the index is SDI index
@param[in]	index	in-memory index structure
@return true if index is SDI index else false */
UNIV_INLINE
bool dict_index_is_sdi(const dict_index_t *index);

/** Check if an table id belongs SDI table
@param[in]	table_id	dict_table_t id
@return true if table_id is SDI table_id else false */
UNIV_INLINE
bool dict_table_is_sdi(uint64_t table_id);

/** Close SDI table.
@param[in]	table		the in-meory SDI table object */
void dict_sdi_close_table(dict_table_t *table);

/** Acquire exclusive MDL on SDI tables. This is acquired to
prevent concurrent DROP table/tablespace when there is purge
happening on SDI table records. Purge will acquired shared
MDL on SDI table.

Exclusive MDL is transactional(released on trx commit). So
for successful acquistion, there should be valid thd with
trx associated.

Acquistion order of SDI MDL and SDI table has to be in same
order:

1. dd_sdi_acquire_exclusive_mdl
2. row_drop_table_from_cache()/innodb_drop_tablespace()
   ->dd_sdi_remove_from_cache()->dd_table_open_on_id()

In purge:

1. dd_sdi_acquire_shared_mdl
2. dd_table_open_on_id()

@param[in]	thd		server thread instance
@param[in]	space_id	InnoDB tablespace id
@param[in,out]	sdi_mdl		MDL ticket on SDI table
@retval	DB_SUCESS		on success
@retval	DB_LOCK_WAIT_TIMEOUT	on error */
dberr_t dd_sdi_acquire_exclusive_mdl(THD *thd, space_id_t space_id,
                                     MDL_ticket **sdi_mdl);

/** Acquire shared MDL on SDI tables. This is acquired by purge to
prevent concurrent DROP table/tablespace.
DROP table/tablespace will acquire exclusive MDL on SDI table

Acquistion order of SDI MDL and SDI table has to be in same
order:

1. dd_sdi_acquire_exclusive_mdl
2. row_drop_table_from_cache()/innodb_drop_tablespace()
   ->dict_sdi_remove_from_cache()->dd_table_open_on_id()

In purge:

1. dd_sdi_acquire_shared_mdl
2. dd_table_open_on_id()

MDL should be released by caller
@param[in]	thd		server thread instance
@param[in]	space_id	InnoDB tablespace id
@param[in,out]	sdi_mdl		MDL ticket on SDI table
@retval	DB_SUCESS		on success
@retval	DB_LOCK_WAIT_TIMEOUT	on error */
dberr_t dd_sdi_acquire_shared_mdl(THD *thd, space_id_t space_id,
                                  MDL_ticket **sdi_mdl);

/** Check whether the dict_table_t is a partition.
A partitioned table on the SQL level is composed of InnoDB tables,
where each InnoDB table is a [sub]partition including its secondary indexes
which belongs to the partition.
@param[in]	table	Table to check.
@return true if the dict_table_t is a partition else false. */
UNIV_INLINE
bool dict_table_is_partition(const dict_table_t *table);

/** Allocate memory for intrinsic cache elements in the index
 * @param[in]      index   index object */
UNIV_INLINE
void dict_allocate_mem_intrinsic_cache(dict_index_t *index);

/** Evict all tables that are loaded for applying purge.
Since we move the offset of all table ids during upgrade,
these tables cannot exist in cache. Also change table_ids
of SYS_* tables if they are upgraded from earlier versions */
void dict_upgrade_evict_tables_cache();

/** @return true if table is InnoDB SYS_* table
@param[in]	table_id	table id  */
bool dict_table_is_system(table_id_t table_id);

/** Build the table_id array of SYS_* tables. This
array is used to determine if a table is InnoDB SYSTEM
table or not.
@return true if successful, false otherwise */
bool dict_sys_table_id_build();

/** Change the table_id of SYS_* tables if they have been created after
an earlier upgrade. This will update the table_id by adding DICT_MAX_DD_TABLES
*/
void dict_table_change_id_sys_tables();

/** Get the tablespace data directory if set, otherwise empty string.
@return the data directory */
std::string dict_table_get_datadir(const dict_table_t *table)
    MY_ATTRIBUTE((warn_unused_result));

#include "dict0dict.ic"

#endif
