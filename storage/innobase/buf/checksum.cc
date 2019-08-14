/*****************************************************************************

Copyright (c) 1995, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file buf/checksum.cc
 Buffer pool checksum functions, also linked from /extra/innochecksum.cc

 Created Aug 11, 2011 Vasil Dimov
 *******************************************************/

#include <innodb/univ/univ.h>

#include <innodb/machine/data.h>
#include <innodb/logger/error.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/log_recv/recv_lsn_checks_on.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/univ/page_no_t.h>
#include <innodb/tablespace/fil_space_t.h>
#include <innodb/page/page_id_t.h>

extern const char *FORCE_RECOVERY_MSG;


/** Do lsn checks on a page during innodb recovery.
@param[in]	check_lsn	if recv_lsn_checks_on & check_lsn
                                perform lsn check
@param[in]	read_buf	buffer containing the page. */
inline void buf_page_lsn_check(bool check_lsn, const byte *read_buf) {
#if !defined(UNIV_HOTBACKUP) && !defined(UNIV_LIBRARY)
  if (check_lsn && recv_lsn_checks_on) {
    lsn_t current_lsn;
    const lsn_t page_lsn = mach_read_from_8(read_buf + FIL_PAGE_LSN);

    /* Since we are going to reset the page LSN during the import
    phase it makes no sense to spam the log with error messages. */
    current_lsn = log_get_lsn(*log_sys);

    if (current_lsn < page_lsn) {
      const space_id_t space_id =
          mach_read_from_4(read_buf + FIL_PAGE_SPACE_ID);
      const page_no_t page_no = mach_read_from_4(read_buf + FIL_PAGE_OFFSET);

      auto space = fil_space_get(space_id);

#ifdef UNIV_NO_ERR_MSGS
      ib::error()
#else
      ib::error(ER_IB_MSG_146)
#endif /* UNIV_NO_ERR_MSGS */
          << "Tablespace '" << space->name << "'"
          << " Page " << page_id_t(space_id, page_no) << " log sequence number "
          << page_lsn << " is in the future! Current system"
          << " log sequence number " << current_lsn << ".";

#ifdef UNIV_NO_ERR_MSGS
      ib::error()
#else
      ib::error(ER_IB_MSG_147)
#endif /* UNIV_NO_ERR_MSGS */
          << "Your database may be corrupt or you may have copied the InnoDB"
          << " tablespace but not the InnoDB log files. " << FORCE_RECOVERY_MSG;
    }
  }
#endif /* !UNIV_HOTBACKUP && !UNIV_LIBRARY */
}

