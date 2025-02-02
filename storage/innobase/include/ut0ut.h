/*****************************************************************************

Copyright (c) 1994, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/ut0ut.h
 Various utilities

 Created 1/20/1994 Heikki Tuuri
 ***********************************************************************/

/**************************************************/ /**
 @page PAGE_INNODB_UTILS Innodb utils

 Useful data structures:
 - @ref Link_buf - to track concurrent operations
 - @ref Sharded_rw_lock - sharded rw-lock (very fast s-lock, slow x-lock)

 *******************************************************/

#ifndef ut0ut_h
#define ut0ut_h


#include <string>
#include <stdio.h>

#include <innodb/univ/univ.h>

/** Index name prefix in fast index creation, as a string constant */
#define TEMP_INDEX_PREFIX_STR "\377"


/* Forward declaration of transaction handle */
struct trx_t;

/** Get a fixed-length string, quoted as an SQL identifier.
If the string contains a slash '/', the string will be
output as two identifiers separated by a period (.),
as in SQL database_name.identifier.
 @param		[in]	trx		transaction (NULL=no quotes).
 @param		[in]	name		table name.
 @retval	String quoted as an SQL identifier.
*/
std::string ut_get_name(const trx_t *trx, const char *name);

/** Outputs a fixed-length string, quoted as an SQL identifier.
 If the string contains a slash '/', the string will be
 output as two identifiers separated by a period (.),
 as in SQL database_name.identifier. */
void ut_print_name(FILE *f,           /*!< in: output stream */
                   const trx_t *trx,  /*!< in: transaction */
                   const char *name); /*!< in: table name to print */

/** Format a table name, quoted as an SQL identifier.
If the name contains a slash '/', the result will contain two
identifiers separated by a period (.), as in SQL
database_name.table_name.
@see table_name_t
@param[in]	name		table or index name
@param[out]	formatted	formatted result, will be NUL-terminated
@param[in]	formatted_size	size of the buffer in bytes
@return pointer to 'formatted' */
char *ut_format_name(const char *name, char *formatted, ulint formatted_size);

/** Catenate files. */
void ut_copy_file(FILE *dest, /*!< in: output file */
                  FILE *src); /*!< in: input file to be appended to output */



#ifdef UNIV_HOTBACKUP
/** Sprintfs a timestamp to a buffer with no spaces and with ':' characters
replaced by '_'.
@param[in]	buf	buffer where to sprintf */
void meb_sprintf_timestamp_without_extra_chars(char *buf);
#endif /* UNIV_HOTBACKUP */


#endif /* !ut0ut_h */
