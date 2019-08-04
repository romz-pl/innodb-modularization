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

/** @file data/data0type.cc
 Data types

 Created 1/16/1996 Heikki Tuuri
 *******************************************************/

#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/assert/assert.h>
#include <innodb/data_types/flags.h>

bool rec_field_not_null_not_add_col_def(ulint len);

ulint innobase_get_at_most_n_mbchars(ulint charset_id, ulint prefix_len, ulint data_len, const char *str);



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
    const char *str)   /*!< in: the string whose prefix
                       length is being determined */
{
  ulint mbminlen = DATA_MBMINLEN(mbminmaxlen);
  ulint mbmaxlen = DATA_MBMAXLEN(mbminmaxlen);

  ut_a(rec_field_not_null_not_add_col_def(data_len));
  ut_ad(!mbmaxlen || !(prefix_len % mbmaxlen));

  if (mbminlen != mbmaxlen) {
    ut_a(!(prefix_len % mbmaxlen));
    return (innobase_get_at_most_n_mbchars(dtype_get_charset_coll(prtype),
                                           prefix_len, data_len, str));
  }

  if (prefix_len < data_len) {
    return (prefix_len);
  }

  return (data_len);
}

