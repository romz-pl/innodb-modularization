/*****************************************************************************

Copyright (c) 1996, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/data0type.h
 Data types

 Created 1/16/1996 Heikki Tuuri
 *******************************************************/

#ifndef data0type_h
#define data0type_h

#include <innodb/univ/univ.h>

#include <innodb/data_types/data_mysql_default_charset_coll.h>
#include <innodb/data_types/dtype_get_mysql_type.h>
#include <innodb/data_types/dtype_is_non_binary_string_type.h>
#include <innodb/data_types/dtype_is_string_type.h>
#include <innodb/data_types/dtype_set.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/dtype_validate.h>
#include <innodb/data_types/flags.h>
#include <innodb/data_types/ib_like_t.h>
#include <innodb/page/flag.h>
#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/data_types/dtype_get_mbmaxlen.h>
#include <innodb/data_types/dtype_get_mbminlen.h>
#include <innodb/data_types/dtype_get_mblen.h>
#include <innodb/data_types/dtype_is_binary_string_type.h>

/* SQL data type struct */
struct dtype_t;



#ifndef UNIV_HOTBACKUP

/** Determine how many bytes the first n characters of the given string occupy.
 If the string is shorter than n characters, returns the number of bytes
 the characters in the string occupy.
 @return length of the prefix, in bytes */
ulint dtype_get_at_most_n_mbchars(
    ulint prtype,      /*!< in: precise type */
    ulint mbminmaxlen, /*!< in: minimum and maximum length of
                       a multi-byte character */
    ulint prefix_len,  /*!< in: length of the requested
                       prefix, in characters, multiplied by
                       dtype_get_mbmaxlen(dtype) */
    ulint data_len,    /*!< in: length of str (in bytes) */
    const char *str);  /*!< in: the string whose prefix
                       length is being determined */
#endif                 /* !UNIV_HOTBACKUP */








/** Copies a data type structure.
@param[in]	type1	type struct to copy to
@param[in]	type2	type struct to copy from */
UNIV_INLINE
void dtype_copy(dtype_t *type1, const dtype_t *type2);

/** Gets the SQL main data type.
 @return SQL main data type */
UNIV_INLINE
ulint dtype_get_mtype(const dtype_t *type); /*!< in: data type */
/** Gets the precise data type.
 @return precise data type */
UNIV_INLINE
ulint dtype_get_prtype(const dtype_t *type); /*!< in: data type */


/** Forms a precise type from the < 4.1.2 format precise type plus the
 charset-collation code.
 @return precise type, including the charset-collation code */
ulint dtype_form_prtype(
    ulint old_prtype,    /*!< in: the MySQL type code and the flags
                         DATA_BINARY_TYPE etc. */
    ulint charset_coll); /*!< in: MySQL charset-collation code */
/** Determines if a MySQL string type is a subset of UTF-8.  This function
 may return false negatives, in case further character-set collation
 codes are introduced in MySQL later.
 @return true if a subset of UTF-8 */
UNIV_INLINE
ibool dtype_is_utf8(ulint prtype); /*!< in: precise data type */
/** Gets the type length.
 @return fixed length of the type, in bytes, or 0 if variable-length */
UNIV_INLINE
ulint dtype_get_len(const dtype_t *type); /*!< in: data type */
#ifndef UNIV_HOTBACKUP


/** Sets the minimum and maximum length of a character, in bytes.
@param[in,out]	type		type
@param[in]	mbminlen	minimum length of a char, in bytes, or 0 if
                                this is not a character type
@param[in]	mbmaxlen	maximum length of a char, in bytes, or 0 if
                                this is not a character type */
UNIV_INLINE
void dtype_set_mbminmaxlen(dtype_t *type, ulint mbminlen, ulint mbmaxlen);
#endif /* !UNIV_HOTBACKUP */

/** Returns the size of a fixed size data type, 0 if not a fixed size type.
@param[in]	mtype		main type
@param[in]	prtype		precise type
@param[in]	len		length
@param[in]	mbminmaxlen	minimum and maximum length of a multibyte
                                character, in bytes
@param[in]	comp		nonzero=ROW_FORMAT=COMPACT
@return fixed size, or 0 */
UNIV_INLINE
ulint dtype_get_fixed_size_low(ulint mtype, ulint prtype, ulint len,
                               ulint mbminmaxlen, ulint comp);

/** Returns the minimum size of a data type.
@param[in]	mtype		main type
@param[in]	prtype		precise type
@param[in]	len		length
@param[in]	mbminmaxlen	minimum and maximum length of a multibyte
                                character, in bytes
@return minimum size */
UNIV_INLINE
ulint dtype_get_min_size_low(ulint mtype, ulint prtype, ulint len,
                             ulint mbminmaxlen);

/** Returns the maximum size of a data type. Note: types in system tables may be
incomplete and return incorrect information.
@param[in]	mtype	main type
@param[in]	len	length
@return maximum size */
UNIV_INLINE
ulint dtype_get_max_size_low(ulint mtype, ulint len);

/** Returns the ROW_FORMAT=REDUNDANT stored SQL NULL size of a type.
For fixed length types it is the fixed length of the type, otherwise 0.
@param[in]	type	type struct
@param[in]	comp	nonzero=ROW_FORMAT=COMPACT
@return SQL null storage size in ROW_FORMAT=REDUNDANT */
UNIV_INLINE
ulint dtype_get_sql_null_size(const dtype_t *type, ulint comp);

#ifndef UNIV_HOTBACKUP
/** Reads to a type the stored information which determines its alphabetical
ordering and the storage size of an SQL NULL value.
@param[in]	type	type struct
@param[in]	buf	buffer for the stored order info */
UNIV_INLINE
void dtype_read_for_order_and_null_size(dtype_t *type, const byte *buf);

/** Stores for a type the information which determines its alphabetical
ordering and the storage size of an SQL NULL value. This is the >= 4.1.x
storage format.
@param[in]	buf		buffer for DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE
                                bytes where we store the info
@param[in]	type		type struct
@param[in]	prefix_len	prefix length to replace type->len, or 0 */
UNIV_INLINE
void dtype_new_store_for_order_and_null_size(byte *buf, const dtype_t *type,
                                             ulint prefix_len);

/** Reads to a type the stored information which determines its alphabetical
ordering and the storage size of an SQL NULL value. This is the 4.1.x storage
format.
@param[in]	type	type struct
@param[in]	buf	buffer for stored type order info */
UNIV_INLINE
void dtype_new_read_for_order_and_null_size(dtype_t *type, const byte *buf);

/** Returns the type's SQL name (e.g. BIGINT UNSIGNED) from mtype,prtype,len
@param[in]	mtype	main type
@param[in]	prtype	precise type
@param[in]	len	length
@param[out]	name	SQL name
@param[in]	name_sz	size of the name buffer
@return the SQL type name */
UNIV_INLINE
char *dtype_sql_name(unsigned mtype, unsigned prtype, unsigned len, char *name,
                     unsigned name_sz);
#endif /* !UNIV_HOTBACKUP */


#ifdef UNIV_DEBUG
/** Print a data type structure.
@param[in]	type	data type */
void dtype_print(const dtype_t *type);
#endif /* UNIV_DEBUG */


static_assert(TRUE == 1, "TRUE != 1");

static_assert(DATA_TRX_ID_LEN == 6, "DATA_TRX_ID_LEN != 6!");

static_assert(DATA_ROLL_PTR_LEN == 7, "DATA_PTR_LEN != 7!");

static_assert(DATA_TRX_ID + 1 == DATA_ROLL_PTR, "DATA_TRX_ID value invalid!");

#include "data0type.ic"

#endif
