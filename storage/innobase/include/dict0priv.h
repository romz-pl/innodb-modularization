/*****************************************************************************

Copyright (c) 2010, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/dict0priv.h
 Data dictionary private functions

 Created  Fri 2 Jul 2010 13:30:38 EST - Sunny Bains
 *******************************************************/

#ifndef dict0priv_h
#define dict0priv_h

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_check_if_in_cache_low.h>
#include <innodb/hash/HASH_SEARCH.h>

#include "dict0dict.h"
#include "dict0load.h"

/** Gets a table; loads it to the dictionary cache if necessary. A low-level
 function. Note: Not to be called from outside dict0*c functions.
 @return table, NULL if not found */
UNIV_INLINE
dict_table_t *dict_table_get_low(const char *table_name); /*!< in: table name */



/** Gets a table; loads it to the dictionary cache if necessary. A low-level
 function.
 @return table, NULL if not found */
UNIV_INLINE
dict_table_t *dict_table_get_low(const char *table_name) /*!< in: table name */
{
  dict_table_t *table;

  ut_ad(table_name);
  ut_ad(mutex_own(&dict_sys->mutex));

  table = dict_table_check_if_in_cache_low(table_name);

  if (table && table->is_corrupted()) {
    ib::error error(ER_IB_MSG_1229);
    error << "Table " << table->name << "is corrupted";
    if (srv_load_corrupted) {
      error << ", but innodb_force_load_corrupted is set";
    } else {
      return (NULL);
    }
  }

  if (table == NULL) {
    table = dict_load_table(table_name, true, DICT_ERR_IGNORE_NONE);
  }

  ut_ad(!table || table->cached);

  return (table);
}


#endif /* dict0priv.h */
