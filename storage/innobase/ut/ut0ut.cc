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

/** @file ut/ut0ut.cc
 Various utilities for Innobase.

 Created 5/11/1994 Heikki Tuuri
 ********************************************************************/

#include "my_config.h"

#include <innodb/error/ut_error.h>
#include <innodb/logger/fatal.h>

#include <errno.h>
#include <time.h>
#include <string>

#include "ha_prototypes.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifndef UNIV_HOTBACKUP
#include <mysql_com.h>
#endif /* !UNIV_HOTBACKUP */

#include "mysql_com.h"

#ifndef UNIV_HOTBACKUP
#include "sql/log.h"

#endif /* !UNIV_HOTBACKUP */

#include "mysql/components/services/log_builtins.h"
#include "sql/derror.h"
#include <innodb/trx_trx/trx_t.h>



#ifdef UNIV_HOTBACKUP
/** Sprintfs a timestamp to a buffer with no spaces and with ':' characters
replaced by '_'.
@param[in]	buf	buffer where to sprintf */
void meb_sprintf_timestamp_without_extra_chars(
    char *buf) /*!< in: buffer where to sprintf */
{
#ifdef _WIN32
  SYSTEMTIME cal_tm;

  GetLocalTime(&cal_tm);

  sprintf(buf, "%02d%02d%02d_%2d_%02d_%02d", (int)cal_tm.wYear % 100,
          (int)cal_tm.wMonth, (int)cal_tm.wDay, (int)cal_tm.wHour,
          (int)cal_tm.wMinute, (int)cal_tm.wSecond);
#else
  struct tm *cal_tm_ptr;
  time_t tm;

  struct tm cal_tm;
  time(&tm);
  localtime_r(&tm, &cal_tm);
  cal_tm_ptr = &cal_tm;
  sprintf(buf, "%02d%02d%02d_%2d_%02d_%02d", cal_tm_ptr->tm_year % 100,
          cal_tm_ptr->tm_mon + 1, cal_tm_ptr->tm_mday, cal_tm_ptr->tm_hour,
          cal_tm_ptr->tm_min, cal_tm_ptr->tm_sec);
#endif
}

#endif /* UNIV_HOTBACKUP */


#ifndef UNIV_HOTBACKUP
/** Get a fixed-length string, quoted as an SQL identifier.
If the string contains a slash '/', the string will be
output as two identifiers separated by a period (.),
as in SQL database_name.identifier.
 @param		[in]	trx		transaction (NULL=no quotes).
 @param		[in]	name		table name.
 @retval	String quoted as an SQL identifier.
*/
std::string ut_get_name(const trx_t *trx, const char *name) {
  /* 2 * NAME_LEN for database and table name,
  and some slack for the #mysql50# prefix and quotes */
  char buf[3 * NAME_LEN];
  const char *bufend;

  bufend = innobase_convert_name(buf, sizeof buf, name, strlen(name),
                                 trx ? trx->mysql_thd : NULL);
  buf[bufend - buf] = '\0';
  return (std::string(buf, 0, bufend - buf));
}

/** Outputs a fixed-length string, quoted as an SQL identifier.
 If the string contains a slash '/', the string will be
 output as two identifiers separated by a period (.),
 as in SQL database_name.identifier. */
void ut_print_name(FILE *f,          /*!< in: output stream */
                   const trx_t *trx, /*!< in: transaction */
                   const char *name) /*!< in: name to print */
{
  /* 2 * NAME_LEN for database and table name,
  and some slack for the #mysql50# prefix and quotes */
  char buf[3 * NAME_LEN];
  const char *bufend;

  bufend = innobase_convert_name(buf, sizeof buf, name, strlen(name),
                                 trx ? trx->mysql_thd : NULL);

  if (fwrite(buf, 1, bufend - buf, f) != (size_t)(bufend - buf)) {
    perror("fwrite");
  }
}

/** Format a table name, quoted as an SQL identifier.
If the name contains a slash '/', the result will contain two
identifiers separated by a period (.), as in SQL
database_name.table_name.
@see table_name_t
@param[in]	name		table or index name
@param[out]	formatted	formatted result, will be NUL-terminated
@param[in]	formatted_size	size of the buffer in bytes
@return pointer to 'formatted' */
char *ut_format_name(const char *name, char *formatted, ulint formatted_size) {
  switch (formatted_size) {
    case 1:
      formatted[0] = '\0';
      /* FALL-THROUGH */
    case 0:
      return (formatted);
  }

  char *end;

  end = innobase_convert_name(formatted, formatted_size, name, strlen(name),
                              NULL);

  /* If the space in 'formatted' was completely used, then sacrifice
  the last character in order to write '\0' at the end. */
  if ((ulint)(end - formatted) == formatted_size) {
    end--;
  }

  ut_a((ulint)(end - formatted) < formatted_size);

  *end = '\0';

  return (formatted);
}

/** Catenate files. */
void ut_copy_file(FILE *dest, /*!< in: output file */
                  FILE *src)  /*!< in: input file to be appended to output */
{
  long len = ftell(src);
  char buf[4096];

  rewind(src);
  do {
    size_t maxs = len < (long)sizeof buf ? (size_t)len : sizeof buf;
    size_t size = fread(buf, 1, maxs, src);
    if (fwrite(buf, 1, size, dest) != size) {
      perror("fwrite");
    }
    len -= (long)size;
    if (size < maxs) {
      break;
    }
  } while (len > 0);
}
#endif /* !UNIV_HOTBACKUP */




namespace ib {

#if !defined(UNIV_HOTBACKUP) && !defined(UNIV_NO_ERR_MSGS)

logger::~logger() {
  auto s = m_oss.str();

  LogEvent()
      .type(LOG_TYPE_ERROR)
      .prio(m_level)
      .errcode(m_err)
      .subsys("InnoDB")
      .verbatim(s.c_str());
}

fatal::~fatal() {
  auto s = m_oss.str();

  LogEvent()
      .type(LOG_TYPE_ERROR)
      .prio(m_level)
      .errcode(m_err)
      .subsys("InnoDB")
      .verbatim(s.c_str());

  ut_error;
}

// fatal_or_error::~fatal_or_error() { ut_a(!m_fatal); }

#endif /* !UNIV_NO_ERR_MSGS */

}  // namespace ib
