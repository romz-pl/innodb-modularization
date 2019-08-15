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
#include <innodb/dict_mem/dict_mem_table_add_v_col.h>
#include <innodb/dict_mem/dict_mem_table_add_s_col.h>
#include <innodb/dict_mem/dict_mem_fill_index_struct.h>
#include <innodb/memory/mem_heap_strdup.h>
#include <innodb/dict_mem/dict_mem_table_col_rename_low.h>
#include <innodb/dict_mem/dict_mem_table_col_rename.h>
#include <innodb/dict_mem/dict_mem_index_free.h>
#include <innodb/dict_mem/dict_mem_foreign_create.h>
#include <innodb/dict_mem/dict_mem_foreign_table_name_lookup_set.h>
#include <innodb/dict_mem/dict_mem_referenced_table_name_lookup_set.h>
#include <innodb/dict_mem/dict_mem_fill_vcol_set_for_base_col.h>
#include <innodb/dict_mem/dict_mem_fill_vcol_from_v_indexes.h>
#include <innodb/dict_mem/dict_mem_fill_vcol_has_index.h>
#include <innodb/dict_mem/dict_mem_table_fill_foreign_vcol_set.h>
#include <innodb/dict_mem/dict_mem_table_free_foreign_vcol_set.h>
#include <innodb/dict_mem/dict_mem_create_temporary_tablename.h>
#include <innodb/dict_mem/dict_mem_init.h>
#include <innodb/random/random.h>
#include <innodb/dict_mem/os_once.h>

#include "dict0mem.h"
#include "fil0fil.h"
#include "buf0flu.h"
#include "sql/dd/object_id.h"
#include "sql/dd/types/column.h"
#include "dict/mem.h"

#include "sql/sql_const.h" /* MAX_KEY_LENGTH */

#include <algorithm>
#include <iterator>
#include <memory> /* std::unique_ptr */
#include <set>
#include <vector>


namespace dd {
class Spatial_reference_system;
}


/** mysql template structure defined in row0mysql.cc */
struct mysql_row_templ_t;
struct lock_t;
struct lock_sys_t;
struct lock_table_t;
struct ib_rbt_t;
struct dict_foreign_t;

#ifdef UNIV_HOTBACKUP
#include "sql/dd/types/spatial_reference_system.h"
#endif /* UNIV_HOTBACKUP */


#include "btr0types.h"
#include "row0types.h"
#include "buf0buf.h"
#include "gis0type.h"


#ifndef UNIV_HOTBACKUP

#include "que0types.h"

#endif /* !UNIV_HOTBACKUP */




#ifndef UNIV_HOTBACKUP
#include "fts0fts.h"
#endif /* !UNIV_HOTBACKUP */



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



#endif /* dict0mem_h */
