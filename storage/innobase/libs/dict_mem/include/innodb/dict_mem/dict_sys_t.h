#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_types/DictSysMutex.h>
#include <innodb/hash/hash_table_t.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/hash/HASH_GET_NEXT.h>
#include <innodb/hash/HASH_GET_FIRST.h>
#include <innodb/dict_mem/dict_table_t.h>

#include "sql/dd/object_id.h"

#include <set>

struct dict_table_t;

/* Dictionary system struct */
struct dict_sys_t {
#ifndef UNIV_HOTBACKUP
  DictSysMutex mutex;          /*!< mutex protecting the data
                               dictionary; protects also the
                               disk-based dictionary system tables;
                               this mutex serializes CREATE TABLE
                               and DROP TABLE, as well as reading
                               the dictionary data for a table from
                               system tables */
#endif                         /* !UNIV_HOTBACKUP */
  row_id_t row_id;             /*!< the next row id to assign;
                               NOTE that at a checkpoint this
                               must be written to the dict system
                               header and flushed to a file; in
                               recovery this must be derived from
                               the log records */
  hash_table_t *table_hash;    /*!< hash table of the tables, based
                               on name */
  hash_table_t *table_id_hash; /*!< hash table of the tables, based
                               on id */
  lint size;                   /*!< varying space in bytes occupied
                               by the data dictionary table and
                               index objects */
  /** Handler to sys_* tables, they're only for upgrade */
  dict_table_t *sys_tables;  /*!< SYS_TABLES table */
  dict_table_t *sys_columns; /*!< SYS_COLUMNS table */
  dict_table_t *sys_indexes; /*!< SYS_INDEXES table */
  dict_table_t *sys_fields;  /*!< SYS_FIELDS table */
  dict_table_t *sys_virtual; /*!< SYS_VIRTUAL table */

  /** Permanent handle to mysql.innodb_table_stats */
  dict_table_t *table_stats;
  /** Permanent handle to mysql.innodb_index_stats */
  dict_table_t *index_stats;
  /** Permanent handle to mysql.innodb_ddl_log */
  dict_table_t *ddl_log;
  /** Permanent handle to mysql.innodb_dynamic_metadata */
  dict_table_t *dynamic_metadata;

  UT_LIST_BASE_NODE_T(dict_table_t)
  table_LRU; /*!< List of tables that can be evicted
             from the cache */
  UT_LIST_BASE_NODE_T(dict_table_t)
  table_non_LRU; /*!< List of tables that can't be
                 evicted from the cache */

  /** Iterate each table.
  @tparam Functor visitor
  @param[in,out]  functor to be invoked on each table */
  template <typename Functor>
  void for_each_table(Functor &functor) {
    mutex_enter(&mutex);

    hash_table_t *hash = table_id_hash;

    for (ulint i = 0; i < hash->n_cells; i++) {
      for (dict_table_t *table =
               static_cast<dict_table_t *>(HASH_GET_FIRST(hash, i));
           table;
           table = static_cast<dict_table_t *>(HASH_GET_NEXT(id_hash, table))) {
        functor(table);
      }
    }

    mutex_exit(&mutex);
  }


  /** Set of ids of DD tables */
  static std::set<dd::Object_id> s_dd_table_ids;

  /** Check if a table is hardcoded. it only includes the dd tables
  @param[in]	id	table ID
  @retval true	if the table is a persistent hard-coded table
                  (dict_table_t::is_temporary() will not hold)
  @retval false	if the table is not hard-coded
                  (it can be persistent or temporary) */
  static bool is_dd_table_id(table_id_t id) {
    return (s_dd_table_ids.find(id) != s_dd_table_ids.end());
  }

  /** The name of the data dictionary tablespace. */
  static const char *s_dd_space_name;

  /** The file name of the data dictionary tablespace. */
  static const char *s_dd_space_file_name;

  /** The name of the hard-coded system tablespace. */
  static const char *s_sys_space_name;

  /** The name of the predefined temporary tablespace. */
  static const char *s_temp_space_name;

  /** The file name of the predefined temporary tablespace. */
  static const char *s_temp_space_file_name;

  /** The hard-coded tablespace name innodb_file_per_table. */
  static const char *s_file_per_table_name;

  /** These two undo tablespaces cannot be dropped. */
  static const char *s_default_undo_space_name_1;
  static const char *s_default_undo_space_name_2;

  /** The table ID of mysql.innodb_dynamic_metadata */
  static constexpr table_id_t s_dynamic_meta_table_id = 2;

  /** The clustered index ID of mysql.innodb_dynamic_metadata */
  static constexpr space_index_t s_dynamic_meta_index_id = 2;
};

