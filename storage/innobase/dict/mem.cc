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

/** @file dict/mem.cc
 Data dictionary memory object creation

 Created 1/8/1996 Heikki Tuuri
 ***********************************************************************/

/** NOTE: The functions in this file should only use functions from
other files in library. The code in this file is used to make a library for
external tools. */

#include <innodb/dict_mem/dict_mem_table_create.h>
#include <innodb/dict_mem/dict_table_mutex_destroy.h>
#include <innodb/dict_mem/dict_index_zip_pad_mutex_create_lazy.h>
#include <innodb/dict_mem/dict_table_mutex_create_lazy.h>
#include <innodb/dict_mem/dict_mem_fill_index_struct.h>
#include <innodb/dict_mem/dict_table_autoinc_create_lazy.h>
#include <innodb/dict_mem/dict_table_autoinc_destroy.h>
#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/data_types/dtype_get_mblen.h>
#include <innodb/string/mem_strdup.h>
#include <innodb/dict_mem/lock_table_lock_list_init.h>


#include <new>

#include "dict0dict.h"
#ifndef UNIV_HOTBACKUP
#include "lock0lock.h"
#endif /* !UNIV_HOTBACKUP */



/** Creates a table memory object.
 @return own: table object */
dict_table_t *dict_mem_table_create(
    const char *name, /*!< in: table name */
    space_id_t space, /*!< in: space where the clustered index of
                      the table is placed */
    ulint n_cols,     /*!< in: total number of columns including
                      virtual and non-virtual columns */
    ulint n_v_cols,   /*!< in: number of virtual columns */
    uint32_t flags,   /*!< in: table flags */
    uint32_t flags2)  /*!< in: table flags2 */
{
  dict_table_t *table;
  mem_heap_t *heap;

  ut_ad(name);
#ifndef UNIV_HOTBACKUP
  ut_a(dict_tf2_is_valid(flags, flags2));
  ut_a(!(flags2 & DICT_TF2_UNUSED_BIT_MASK));
#endif /* !UNIV_HOTBACKUP */

  heap = mem_heap_create(DICT_HEAP_SIZE);

  table = static_cast<dict_table_t *>(mem_heap_zalloc(heap, sizeof(*table)));

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY
  lock_table_lock_list_init(&table->locks);
#endif /* !UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */

  UT_LIST_INIT(table->indexes, &dict_index_t::indexes);

  table->heap = heap;

  ut_d(table->magic_n = DICT_TABLE_MAGIC_N);

  table->flags = (unsigned int)flags;
  table->flags2 = (unsigned int)flags2;
  table->name.m_name = mem_strdup(name);
  table->space = (unsigned int)space;
  table->dd_space_id = dd::INVALID_OBJECT_ID;
  table->n_t_cols = (unsigned int)(n_cols + table->get_n_sys_cols());
  table->n_v_cols = (unsigned int)(n_v_cols);
  table->n_cols = table->n_t_cols - table->n_v_cols;
  table->n_instant_cols = table->n_cols;

  table->cols = static_cast<dict_col_t *>(
      mem_heap_alloc(heap, table->n_cols * sizeof(dict_col_t)));
  table->v_cols = static_cast<dict_v_col_t *>(
      mem_heap_alloc(heap, n_v_cols * sizeof(*table->v_cols)));

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY
  dict_table_mutex_create_lazy(table);

  /* true means that the stats latch will be enabled -
  dict_table_stats_lock() will not be noop. */
  dict_table_stats_latch_create(table, true);

  table->autoinc_lock =
      static_cast<lock_t *>(mem_heap_alloc(heap, lock_get_size()));

  /* lazy creation of table autoinc latch */
  dict_table_autoinc_create_lazy(table);

  table->version = 0;
  table->autoinc = 0;
  table->autoinc_persisted = 0;
  table->autoinc_field_no = ULINT_UNDEFINED;
  table->sess_row_id = 0;
  table->sess_trx_id = 0;

  /* If the table has an FTS index or we are in the process
  of building one, create the table->fts */
  if (dict_table_has_fts_index(table) ||
      DICT_TF2_FLAG_IS_SET(table, DICT_TF2_FTS_HAS_DOC_ID) ||
      DICT_TF2_FLAG_IS_SET(table, DICT_TF2_FTS_ADD_DOC_ID)) {
    table->fts = fts_create(table);
    table->fts->cache = fts_cache_create(table);
  } else {
    table->fts = NULL;
  }

  if (DICT_TF_HAS_SHARED_SPACE(table->flags)) {
    dict_get_and_save_space_name(table, true);
  }

  new (&table->foreign_set) dict_foreign_set();
  new (&table->referenced_set) dict_foreign_set();

#endif /* !UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */
  table->is_dd_table = false;
  table->explicitly_non_lru = false;

  return (table);
}
