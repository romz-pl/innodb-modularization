/*****************************************************************************

Copyright (c) 1994, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/data0data.h
 SQL data field and tuple

 Created 5/30/1994 Heikki Tuuri
 *************************************************************************/

#ifndef data0data_h
#define data0data_h

#include <innodb/univ/univ.h>

#include <innodb/data_types/DTUPLE_EST_ALLOC.h>
#include <innodb/data_types/big_rec_field_t.h>
#include <innodb/data_types/big_rec_t.h>
#include <innodb/data_types/dfield_check_typed.h>
#include <innodb/data_types/dfield_copy.h>
#include <innodb/data_types/dfield_copy_data.h>
#include <innodb/data_types/dfield_data_is_binary_equal.h>
#include <innodb/data_types/dfield_datas_are_binary_equal.h>
#include <innodb/data_types/dfield_dup.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dfield_is_ext.h>
#include <innodb/data_types/dfield_is_null.h>
#include <innodb/data_types/dfield_print.h>
#include <innodb/data_types/dfield_print_also_hex.h>
#include <innodb/data_types/dfield_set_data.h>
#include <innodb/data_types/dfield_set_len.h>
#include <innodb/data_types/dfield_set_null.h>
#include <innodb/data_types/dfield_set_type.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dfield_write_mbr.h>
#include <innodb/data_types/dtuple_big_rec_free.h>
#include <innodb/data_types/dtuple_check_typed.h>
#include <innodb/data_types/dtuple_coll_eq.h>
#include <innodb/data_types/dtuple_contains_null.h>
#include <innodb/data_types/dtuple_copy.h>
#include <innodb/data_types/dtuple_copy_v_fields.h>
#include <innodb/data_types/dtuple_create.h>
#include <innodb/data_types/dtuple_create_from_mem.h>
#include <innodb/data_types/dtuple_create_with_vcol.h>
#include <innodb/data_types/dtuple_dup_v_fld.h>
#include <innodb/data_types/dtuple_fold.h>


#include "btr0types.h"
#include "trx0types.h"

#include <ostream>

struct dfield_t;
struct dtuple_t;
struct dict_index_t;
struct dict_v_col_t;
struct big_rec_t;
struct upd_t;







/** Moves parts of long fields in entry to the big record vector so that
 the size of tuple drops below the maximum record size allowed in the
 database. Moves data only from those fields which are not necessary
 to determine uniquely the insertion place of the tuple in the index.
 @return own: created big record vector, NULL if we are not able to
 shorten the entry enough, i.e., if there are too many fixed-length or
 short fields in entry or the index is clustered */
big_rec_t *dtuple_convert_big_rec(dict_index_t *index, /*!< in: index */
                                  upd_t *upd,      /*!< in/out: update vector */
                                  dtuple_t *entry, /*!< in/out: index entry */
                                  ulint *n_ext)    /*!< in/out: number of
                                                   externally stored columns */
    MY_ATTRIBUTE((malloc, warn_unused_result));


/** Puts back to entry the data stored in vector. Note that to ensure the
 fields in entry can accommodate the data, vector must have been created
 from entry with dtuple_convert_big_rec. */
void dtuple_convert_back_big_rec(
    dict_index_t *index, /*!< in: index */
    dtuple_t *entry,     /*!< in: entry whose data was put to vector */
    big_rec_t *vector);  /*!< in, own: big rec vector; it is
                 freed in this function */



#endif
