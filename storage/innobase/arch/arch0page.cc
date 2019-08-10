/*****************************************************************************

Copyright (c) 2017, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file arch/arch0page.cc
 Innodb implementation for page archive

 *******************************************************/

#include <innodb/ioasync/srv_shutdown_state.h>
#include <innodb/logger/warn.h>
#include <innodb/log_arch/page_archiver_thread_event.h>
#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/log_arch/Page_Wait_Flush_Archiver_Cbk.h>

#include "clone0clone.h"



bool wait_flush_archiver(Page_Wait_Flush_Archiver_Cbk cbk_func) {
  ut_ad(mutex_own(arch_page_sys->get_oper_mutex()));

  while (cbk_func()) {
    /* Need to wait for flush. We don't expect it
    to happen normally. With no duplicate page ID
    dirty page growth should be very slow. */
    os_event_set(page_archiver_thread_event);

    bool is_timeout = false;
    int alert_count = 0;

    auto err = Clone_Sys::wait_default(
        [&](bool alert, bool &result) {

          ut_ad(mutex_own(arch_page_sys->get_oper_mutex()));
          result = cbk_func();

          int err2 = 0;
          if (srv_shutdown_state == SRV_SHUTDOWN_LAST_PHASE ||
              srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS ||
              arch_page_sys->is_abort()) {
            err2 = ER_QUERY_INTERRUPTED;

          } else if (result) {
            os_event_set(page_archiver_thread_event);
            if (alert && ++alert_count == 12) {
              alert_count = 0;
              ib::info(ER_IB_MSG_22) << "Clone Page Tracking: waiting "
                                        "for block to flush";
            }
          }
          return (err2);
        },
        arch_page_sys->get_oper_mutex(), is_timeout);

    if (err != 0) {
      return (false);

    } else if (is_timeout) {
      ut_ad(false);
      ib::warn(ER_IB_MSG_22) << "Clone Page Tracking: wait for block flush "
                                "timed out";
      return (false);
    }
  }
  return (true);
}





/** Wait for archive system to come out of #ARCH_STATE_PREPARE_IDLE.
If the system is preparing to idle, #start needs to wait
for it to come to idle state.
@return true, if successful
        false, if needs to abort */
bool Arch_Page_Sys::wait_idle() {
  ut_ad(mutex_own(&m_mutex));

  if (m_state == ARCH_STATE_PREPARE_IDLE) {
    os_event_set(page_archiver_thread_event);
    bool is_timeout = false;
    int alert_count = 0;

    auto err = Clone_Sys::wait_default(
        [&](bool alert, bool &result) {

          ut_ad(mutex_own(&m_mutex));
          result = (m_state == ARCH_STATE_PREPARE_IDLE);

          if (srv_shutdown_state != SRV_SHUTDOWN_NONE) {
            return (ER_QUERY_INTERRUPTED);
          }

          if (result) {
            os_event_set(page_archiver_thread_event);

            /* Print messages every 1 minute - default is 5 seconds. */
            if (alert && ++alert_count == 12) {
              alert_count = 0;
              ib::info(ER_IB_MSG_24) << "Page Tracking start: waiting for "
                                        "idle state.";
            }
          }
          return (0);
        },
        &m_mutex, is_timeout);

    if (err == 0 && is_timeout) {
      ut_ad(false);
      err = ER_INTERNAL_ERROR;
      ib::info(ER_IB_MSG_25) << "Page Tracking start: wait for idle state "
                                "timed out";
    }

    if (err != 0) {
      return (false);
    }
  }
  return (true);
}

