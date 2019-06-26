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

/** @file include/ut0mem.h
 Memory primitives

 Created 5/30/1994 Heikki Tuuri
 ************************************************************************/

#ifndef ut0mem_h
#define ut0mem_h

#include <innodb/univ/univ.h>

/********************************************************************
Concatenate 3 strings.*/
char *ut_str3cat(
    /* out, own: concatenated string, must be
    freed with ut_free() */
    const char *s1,  /* in: string 1 */
    const char *s2,  /* in: string 2 */
    const char *s3); /* in: string 3 */

/** Converts a raw binary data to a NUL-terminated hex string. The output is
truncated if there is not enough space in "hex", make sure "hex_size" is at
least (2 * raw_size + 1) if you do not want this to happen. Returns the actual
number of characters written to "hex" (including the NUL).
@param[in]	raw		raw data
@param[in]	raw_size	"raw" length in bytes
@param[out]	hex		hex string
@param[in]	hex_size	"hex" size in bytes
@return number of chars written */
UNIV_INLINE
ulint ut_raw_to_hex(const void *raw, ulint raw_size, char *hex, ulint hex_size);

/** Adds single quotes to the start and end of string and escapes any quotes by
doubling them. Returns the number of bytes that were written to "buf"
(including the terminating NUL). If buf_size is too small then the trailing
bytes from "str" are discarded.
@param[in]	str		string
@param[in]	str_len		string length in bytes
@param[out]	buf		output buffer
@param[in]	buf_size	output buffer size in bytes
@return number of bytes that were written */
UNIV_INLINE
ulint ut_str_sql_format(const char *str, ulint str_len, char *buf,
                        ulint buf_size);

#include "ut0mem.ic"

#endif
