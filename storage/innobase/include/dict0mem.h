/*****************************************************************************

Copyright (c) 1996, 2018, Oracle and/or its affiliates. All Rights Reserved.
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

/** @file include/dict0mem.h
 Data dictionary memory object creation

 Created 1/8/1996 Heikki Tuuri
 *******************************************************/

#ifndef dict0mem_h
#define dict0mem_h

#include <innodb/univ/univ.h>

#include <innodb/lock_types/lock_mode.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/data_types/flags.h>
#include <innodb/dict_types/index_id_t.h>
#include <innodb/dict_types/ib_quiesce_t.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/spatial_status_t.h>
#include <innodb/mtr/mtr_commit.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/hash/hash_node_t.h>
#include <innodb/data_types/dtype_get_fixed_size_low.h>
#include <innodb/data_types/dtype_get_min_size_low.h>
#include <innodb/string/ut_strcmp.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/sync_mutex/mutex_destroy.h>
#include <innodb/univ/rec_t.h>
#include <innodb/dict_types/id_name_t.h>
#include <innodb/dict_types/table_name_t.h>
#include <innodb/data_types/dtype_get_max_size_low.h>
#include <innodb/dict_mem/dict_foreign_compare.h>
#include <innodb/dict_mem/dict_table_mutex_destroy.h>
#include <innodb/dict_mem/dict_col_default_t.h>
#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/dict_mem/dict_v_idx_t.h>
#include <innodb/dict_mem/dict_v_idx_list.h>
#include <innodb/dict_mem/dict_v_col_t.h>
#include <innodb/dict_mem/dict_add_v_col_t.h>
#include <innodb/dict_mem/dict_s_col_t.h>
#include <innodb/dict_mem/dict_s_col_list.h>
#include <innodb/dict_mem/dict_field_t.h>
#include <innodb/dict_mem/zip_pad_info_t.h>
#include <innodb/dict_mem/rec_cache_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_mutex_create_lazy.h>
#include <innodb/dict_mem/online_index_status.h>
#include <innodb/dict_mem/dict_vcol_set.h>
#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/dict_mem/dict_foreign_with_index.h>
#include <innodb/dict_mem/dict_foreign_different_tables.h>
#include <innodb/dict_mem/dict_foreign_matches_id.h>
#include <innodb/dict_mem/dict_foreign_set.h>
#include <innodb/dict_mem/dict_foreign_not_exists.h>
#include <innodb/dict_mem/dict_foreign_set_free.h>
#include <innodb/dict_mem/dict_foreign_set_validate.h>
#include <innodb/dict_mem/dict_vcol_templ_t.h>
#include <innodb/dict_mem/table_dirty_status.h>
#include <innodb/dict_mem/temp_prebuilt_vec.h>
#include <innodb/dict_mem/table_lock_list_t.h>
#include <innodb/dict_mem/persistent_type_t.h>
#include <innodb/dict_mem/corrupted_ids_t.h>
#include <innodb/dict_mem/PersistentTableMetadata.h>
#include <innodb/dict_mem/Persisters.h>
#include <innodb/dict_mem/dict_foreign_add_to_referenced_table.h>
#include <innodb/dict_mem/Persister.h>
#include <innodb/dict_mem/CorruptedIndexPersister.h>
#include <innodb/dict_mem/AutoIncPersister.h>
#include <innodb/dict_mem/dict_table_autoinc_destroy.h>
#include <innodb/dict_mem/dict_table_autoinc_create_lazy.h>
#include <innodb/dict_mem/dict_index_zip_pad_mutex_create_lazy.h>
#include <innodb/dict_mem/dict_index_zip_pad_mutex_destroy.h>
#include <innodb/dict_mem/dict_index_zip_pad_unlock.h>
#include <innodb/dict_mem/dict_table_autoinc_own.h>

#include "buf0flu.h"

#include "sql/dd/object_id.h"
#include "sql/dd/types/column.h"
#ifdef UNIV_HOTBACKUP
#include "sql/dd/types/spatial_reference_system.h"
#endif /* UNIV_HOTBACKUP */
#include "btr0types.h"




#include "row0types.h"
#ifndef UNIV_HOTBACKUP


struct lock_t;
struct lock_sys_t;
struct lock_table_t;

#include "que0types.h"
#endif /* !UNIV_HOTBACKUP */


#include <innodb/random/random.h>
#ifndef UNIV_HOTBACKUP
#include "fts0fts.h"
#endif /* !UNIV_HOTBACKUP */
#include "buf0buf.h"
#include "gis0type.h"
#ifndef UNIV_HOTBACKUP
#include <innodb/dict_mem/os_once.h>
#endif /* !UNIV_HOTBACKUP */
#include "dict/mem.h"

#include "sql/sql_const.h" /* MAX_KEY_LENGTH */

#include <algorithm>
#include <iterator>
#include <memory> /* std::unique_ptr */
#include <set>
#include <vector>

/* Forward declaration. */
struct ib_rbt_t;
struct dict_foreign_t;


/** Adds a virtual column definition to a table.
@param[in,out]	table		table
@param[in]	heap		temporary memory heap, or NULL. It is
                                used to store name when we have not finished
                                adding all columns. When all columns are
                                added, the whole name will copy to memory from
                                table->heap
@param[in]	name		column name
@param[in]	mtype		main datatype
@param[in]	prtype		precise type
@param[in]	len		length
@param[in]	pos		position in a table
@param[in]	num_base	number of base columns
@return the virtual column definition */
dict_v_col_t *dict_mem_table_add_v_col(dict_table_t *table, mem_heap_t *heap,
                                       const char *name, ulint mtype,
                                       ulint prtype, ulint len, ulint pos,
                                       ulint num_base);

/** Adds a stored column definition to a table.
@param[in,out]	table		table
@param[in]	num_base	number of base columns. */
void dict_mem_table_add_s_col(dict_table_t *table, ulint num_base);

/** Renames a column of a table in the data dictionary cache. */
void dict_mem_table_col_rename(dict_table_t *table, /*!< in/out: table */
                               ulint nth_col,       /*!< in: column index */
                               const char *from,    /*!< in: old column name */
                               const char *to,      /*!< in: new column name */
                               bool is_virtual);
/*!< in: if this is a virtual column */

/** This function poplulates a dict_index_t index memory structure with
supplied information.
@param[out]	index		index to be filled
@param[in]	heap		memory heap
@param[in]	table_name	table name
@param[in]	index_name	index name
@param[in]	space		space where the index tree is placed, the
                                clustered type ignored if the index is of the
                                clustered type
@param[in]	type		DICT_UNIQUE, DICT_CLUSTERED, ... ORed
@param[in]	n_fields	number of fields */
UNIV_INLINE
void dict_mem_fill_index_struct(dict_index_t *index, mem_heap_t *heap,
                                const char *table_name, const char *index_name,
                                ulint space, ulint type, ulint n_fields);

/** Frees an index memory object. */
void dict_mem_index_free(dict_index_t *index); /*!< in: index */
/** Creates and initializes a foreign constraint memory object.
 @return own: foreign constraint struct */
dict_foreign_t *dict_mem_foreign_create(void);

/** Sets the foreign_table_name_lookup pointer based on the value of
 lower_case_table_names.  If that is 0 or 1, foreign_table_name_lookup
 will point to foreign_table_name.  If 2, then another string is
 allocated from the heap and set to lower case. */
void dict_mem_foreign_table_name_lookup_set(
    dict_foreign_t *foreign, /*!< in/out: foreign struct */
    ibool do_alloc);         /*!< in: is an alloc needed */

/** Sets the referenced_table_name_lookup pointer based on the value of
 lower_case_table_names.  If that is 0 or 1, referenced_table_name_lookup
 will point to referenced_table_name.  If 2, then another string is
 allocated from the heap and set to lower case. */
void dict_mem_referenced_table_name_lookup_set(
    dict_foreign_t *foreign, /*!< in/out: foreign struct */
    ibool do_alloc);         /*!< in: is an alloc needed */

/** Fills the dependent virtual columns in a set.
Reason for being dependent are
1) FK can be present on base column of virtual columns
2) FK can be present on column which is a part of virtual index
@param[in,out]	foreign	foreign key information. */
void dict_mem_foreign_fill_vcol_set(dict_foreign_t *foreign);

/** Fill virtual columns set in each fk constraint present in the table.
@param[in,out]	table	innodb table object. */
void dict_mem_table_fill_foreign_vcol_set(dict_table_t *table);

/** Free the vcol_set from all foreign key constraint on the table.
@param[in,out]	table	innodb table object. */
void dict_mem_table_free_foreign_vcol_set(dict_table_t *table);

/** Create a temporary tablename like "#sql-ibtid-inc" where
  tid = the Table ID
  inc = a randomly initialized number that is incremented for each file
The table ID is a 64 bit integer, can use up to 20 digits, and is
initialized at bootstrap. The second number is 32 bits, can use up to 10
digits, and is initialized at startup to a randomly distributed number.
It is hoped that the combination of these two numbers will provide a
reasonably unique temporary file name.
@param[in]	heap	A memory heap
@param[in]	dbtab	Table name in the form database/table name
@param[in]	id	Table id
@return A unique temporary tablename suitable for InnoDB use */
char *dict_mem_create_temporary_tablename(mem_heap_t *heap, const char *dbtab,
                                          table_id_t id);

/** Initialize dict memory variables */
void dict_mem_init(void);






/** Percentage of compression failures that are allowed in a single
round */
extern ulong zip_failure_threshold_pct;

/** Maximum percentage of a page that can be allowed as a pad to avoid
compression failures */
extern ulong zip_pad_max;



/** Cache position of last inserted or selected record by caching record
and holding reference to the block where record resides.
Note: We don't commit mtr and hold it beyond a transaction lifetime as this is
a special case (intrinsic table) that are not shared accross connection. */
class last_ops_cur_t {
 public:
  /** Constructor */
  last_ops_cur_t() : rec(), block(), mtr(), disable_caching(), invalid() {
    /* Do Nothing. */
  }

  /* Commit mtr and re-initialize cache record and block to NULL. */
  void release() {
    if (mtr.is_active()) {
      mtr_commit(&mtr);
    }
    rec = NULL;
    block = NULL;
    invalid = false;
  }

 public:
  /** last inserted/selected record. */
  rec_t *rec;

  /** block where record reside. */
  buf_block_t *block;

  /** active mtr that will be re-used for next insert/select. */
  mtr_t mtr;

  /** disable caching. (disabled when table involves blob/text.) */
  bool disable_caching;

  /** If index structure is undergoing structural change viz.
  split then invalidate the cached position as it would be no more
  remain valid. Will be re-cached on post-split insert. */
  bool invalid;
};

/** "GEN_CLUST_INDEX" is the name reserved for InnoDB default
system clustered index when there is no primary key. */
const char innobase_index_reserve_name[] = "GEN_CLUST_INDEX";

namespace dd {
class Spatial_reference_system;
}





/** Validate the search order in the foreign key sets of the table
(foreign_set and referenced_set).
@param[in]	table	table whose foreign key sets are to be validated
@return true if foreign key sets are fine, false otherwise. */
bool dict_foreign_set_validate(const dict_table_t &table);






/** mysql template structure defined in row0mysql.cc */
struct mysql_row_templ_t;








#include "dict0mem.ic"

#endif /* dict0mem_h */
