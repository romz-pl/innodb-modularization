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

#pragma once

#include <innodb/univ/univ.h>


#ifndef UNIV_HOTBACKUP

struct dtype_t;

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


