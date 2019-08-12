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
#include <innodb/data_types/dtype_t.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/data_types/spatial_status_t.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/big_rec_field_t.h>
#include <innodb/data_types/big_rec_t.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_set_type.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_set_len.h>
#include <innodb/data_types/dfield_is_null.h>
#include <innodb/data_types/dfield_is_ext.h>
#include <innodb/data_types/dfield_set_data.h>
#include <innodb/data_types/dfield_write_mbr.h>
#include <innodb/data_types/dfield_set_null.h>
#include <innodb/data_types/dfield_copy_data.h>
#include <innodb/data_types/dfield_copy.h>
#include <innodb/data_types/dfield_dup.h>
#include <innodb/data_types/dfield_datas_are_binary_equal.h>
#include <innodb/data_types/dfield_data_is_binary_equal.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_n_v_fields.h>
#include <innodb/data_types/dtuple_get_nth_v_field.h>
#include <innodb/data_types/dtuple_get_info_bits.h>
#include <innodb/data_types/dtuple_set_info_bits.h>
#include <innodb/data_types/dtuple_get_n_fields_cmp.h>
#include <innodb/data_types/dtuple_set_n_fields_cmp.h>
#include <innodb/data_types/dtuple_get_nth_v_field.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/DTUPLE_EST_ALLOC.h>
#include <innodb/data_types/dtuple_create_from_mem.h>
#include <innodb/data_types/dtuple_create.h>
#include <innodb/data_types/dtuple_dup_v_fld.h>
#include <innodb/data_types/dtuple_init_v_fld.h>
#include <innodb/data_types/dtuple_create_with_vcol.h>

#include "trx0types.h"
#include <ostream>

struct dfield_t;
struct dtuple_t;
struct dict_index_t;
struct dict_v_col_t;
struct big_rec_t;
struct upd_t;








/** Sets number of fields used in a tuple. Normally this is set in
 dtuple_create, but if you want later to set it smaller, you can use this. */
void dtuple_set_n_fields(dtuple_t *tuple, /*!< in: tuple */
                         ulint n_fields); /*!< in: number of fields */
/** Copies a data tuple's virtaul fields to another. This is a shallow copy;
@param[in,out]	d_tuple		destination tuple
@param[in]	s_tuple		source tuple */
UNIV_INLINE
void dtuple_copy_v_fields(dtuple_t *d_tuple, const dtuple_t *s_tuple);
/** Copies a data tuple to another.  This is a shallow copy; if a deep copy
 is desired, dfield_dup() will have to be invoked on each field.
 @return own: copy of tuple */
UNIV_INLINE
dtuple_t *dtuple_copy(const dtuple_t *tuple, /*!< in: tuple to copy from */
                      mem_heap_t *heap)      /*!< in: memory heap
                                             where the tuple is created */
    MY_ATTRIBUTE((malloc));

/** The following function returns the sum of data lengths of a tuple. The space
occupied by the field structs or the tuple struct is not counted.
@param[in]	tuple	typed data tuple
@param[in]	comp	nonzero=ROW_FORMAT=COMPACT
@return sum of data lens */
UNIV_INLINE
ulint dtuple_get_data_size(const dtuple_t *tuple, ulint comp);
/** Computes the number of externally stored fields in a data tuple.
 @return number of fields */
UNIV_INLINE
ulint dtuple_get_n_ext(const dtuple_t *tuple); /*!< in: tuple */
/** Compare two data tuples.
@param[in] tuple1 first data tuple
@param[in] tuple2 second data tuple
@return whether tuple1==tuple2 */
bool dtuple_coll_eq(const dtuple_t *tuple1, const dtuple_t *tuple2)
    MY_ATTRIBUTE((warn_unused_result));

/** Compute a hash value of a prefix of an index record.
@param[in]	tuple		index record
@param[in]	n_fields	number of fields to include
@param[in]	n_bytes		number of bytes to fold in the last field
@param[in]	fold		fold value of the index identifier
@return the folded value */
UNIV_INLINE
ulint dtuple_fold(const dtuple_t *tuple, ulint n_fields, ulint n_bytes,
                  ulint fold) MY_ATTRIBUTE((warn_unused_result));

/** Sets types of fields binary in a tuple.
@param[in]	tuple	data tuple
@param[in]	n	number of fields to set */
UNIV_INLINE
void dtuple_set_types_binary(dtuple_t *tuple, ulint n);

/** Checks if a dtuple contains an SQL null value.
 @return true if some field is SQL null */
UNIV_INLINE
ibool dtuple_contains_null(const dtuple_t *tuple) /*!< in: dtuple */
    MY_ATTRIBUTE((warn_unused_result));
/** Checks that a data field is typed. Asserts an error if not.
 @return true if ok */
ibool dfield_check_typed(const dfield_t *field) /*!< in: data field */
    MY_ATTRIBUTE((warn_unused_result));
/** Checks that a data tuple is typed. Asserts an error if not.
 @return true if ok */
ibool dtuple_check_typed(const dtuple_t *tuple) /*!< in: tuple */
    MY_ATTRIBUTE((warn_unused_result));
#ifdef UNIV_DEBUG
/** Validates the consistency of a tuple which must be complete, i.e,
 all fields must have been set.
 @return true if ok */
ibool dtuple_validate(const dtuple_t *tuple) /*!< in: tuple */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* UNIV_DEBUG */
/** Pretty prints a dfield value according to its data type. Also the hex string
 is printed if a string contains non-printable characters. */
void dfield_print_also_hex(const dfield_t *dfield); /*!< in: dfield */
/** The following function prints the contents of a tuple. */
void dtuple_print(FILE *f,                /*!< in: output stream */
                  const dtuple_t *tuple); /*!< in: tuple */

/** Print the contents of a tuple.
@param[out]	o	output stream
@param[in]	field	array of data fields
@param[in]	n	number of data fields */
void dfield_print(std::ostream &o, const dfield_t *field, ulint n);

/** Print the contents of a tuple.
@param[out]	o	output stream
@param[in]	tuple	data tuple */
void dtuple_print(std::ostream &o, const dtuple_t *tuple);

/** Print the contents of a tuple.
@param[out]	o	output stream
@param[in]	tuple	data tuple */
inline std::ostream &operator<<(std::ostream &o, const dtuple_t &tuple) {
  dtuple_print(o, &tuple);
  return (o);
}

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
/** Frees the memory in a big rec vector. */
UNIV_INLINE
void dtuple_big_rec_free(big_rec_t *vector); /*!< in, own: big rec vector; it is
                                     freed in this function */

/*######################################################################*/










#include "data0data.ic"

#endif
