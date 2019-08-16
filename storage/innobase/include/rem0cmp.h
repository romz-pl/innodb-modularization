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

/** @file include/rem0cmp.h
 Comparison services for records

 Created 7/1/1994 Heikki Tuuri
 ************************************************************************/

#ifndef rem0cmp_h
#define rem0cmp_h

#include <innodb/univ/univ.h>

#include <innodb/cmp/cmp_cols_are_equal.h>
#include <innodb/cmp/cmp_data.h>
#include <innodb/cmp/cmp_data_data.h>
#include <innodb/cmp/cmp_decimal.h>
#include <innodb/cmp/cmp_dfield_dfield.h>
#include <innodb/cmp/cmp_dfield_dfield_eq_prefix.h>
#include <innodb/cmp/cmp_dtuple_is_prefix_of_rec.h>
#include <innodb/cmp/cmp_dtuple_rec.h>
#include <innodb/cmp/cmp_dtuple_rec_with_match.h>
#include <innodb/cmp/cmp_dtuple_rec_with_match_low.h>
#include <innodb/cmp/cmp_geometry_field.h>
#include <innodb/cmp/cmp_get_pad_char.h>
#include <innodb/cmp/cmp_gis_field.h>
#include <innodb/cmp/cmp_rec_rec.h>
#include <innodb/cmp/cmp_rec_rec_simple.h>
#include <innodb/cmp/cmp_rec_rec_simple_field.h>
#include <innodb/cmp/cmp_rec_rec_with_match.h>
#include <innodb/cmp/cmp_whole_field.h>
#include <innodb/cmp/innobase_mysql_cmp.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dtuple_get_n_fields_cmp.h>
#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/logger/fatal.h>




#include "btr0types.h"
#include "dict0dict.h"
#include "field_types.h"  // enum_field_types
#include "fts0fts.h"
#include "ha_prototypes.h"
#include <my_sys.h>
#include <mysql_com.h>


namespace dd {
class Spatial_reference_system;
}

#endif
