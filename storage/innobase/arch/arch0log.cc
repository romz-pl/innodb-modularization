/*****************************************************************************

Copyright (c) 2017, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file arch/arch0log.cc
 Innodb implementation for log archive

 *******************************************************/


#include <innodb/ioasync/srv_shutdown_state.h>
#include <innodb/log_arch/log_archiver_thread_event.h>
#include <innodb/log_arch/Arch_Log_Sys.h>

#include "clone0clone.h"



/** Wait for redo log archive up to the target LSN.
We need to wait till current log sys LSN during archive stop.
@param[in]	target_lsn	target archive LSN to wait for
@return error code */
int Arch_Log_Sys::wait_archive_complete(lsn_t target_lsn) {
  target_lsn = ut_uint64_align_down(target_lsn, OS_FILE_LOG_BLOCK_SIZE);

  /* Check and wait for archiver thread if needed. */
  if (m_archived_lsn.load() < target_lsn) {
    os_event_set(log_archiver_thread_event);

    bool is_timeout = false;
    int alert_count = 0;

    auto err = Clone_Sys::wait_default(
        [&](bool alert, bool &result) {

          int err2 = 0;
          /* Check if archived LSN is behind target. */
          auto archived_lsn = m_archived_lsn.load();
          result = (archived_lsn < target_lsn);

          /* Trigger flush if needed */
          auto flush = log_sys->write_lsn.load() < target_lsn;

          /* Check if we need to abort. */
          if (m_state == ARCH_STATE_ABORT ||
              srv_shutdown_state != SRV_SHUTDOWN_NONE) {
            err2 = ER_QUERY_INTERRUPTED;
          }
          ut_ad(m_state == ARCH_STATE_ACTIVE);

          if (err2 != 0) {
            my_error(ER_QUERY_INTERRUPTED, MYF(0));

          } else if (result) {
            /* More data needs to be archived. */
            os_event_set(log_archiver_thread_event);

            /* Write system redo log if needed. */
            if (flush) {
              log_write_up_to(*log_sys, target_lsn, false);
            }
            /* Print messages every 1 minute - default is 5 seconds. */
            if (alert && ++alert_count == 12) {
              alert_count = 0;
              ib::info(ER_IB_MSG_18)
                  << "Clone Log archive stop: waiting for archiver to "
                     "finish archiving log till LSN: "
                  << target_lsn << " Archived LSN: " << archived_lsn;
            }
          }
          return (err2);
        },
        nullptr, is_timeout);

    if (err == 0 && is_timeout) {
      ut_ad(false);

      ib::info(ER_IB_MSG_19) << "Clone Log archive stop: "
                                "wait for Archiver timed out";

      err = ER_INTERNAL_ERROR;
      my_error(ER_INTERNAL_ERROR, MYF(0), "Clone: Log Archiver wait too long");
    }
    return (err);
  }
  return (0);
}
