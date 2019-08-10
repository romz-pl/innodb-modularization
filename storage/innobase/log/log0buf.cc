/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/**************************************************/ /**
 @file log/log0buf.cc

 Redo log buffer implementation, including functions to:

 -# Reserve space in the redo log buffer,

 -# Write to the reserved space in the log buffer,

 -# Add link to the log recent written buffer,

 -# Add link to the log recent closed buffer.

 *******************************************************/

#include <innodb/log_types/log_checkpointer_mutex_exit.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_types/log_checkpointer_mutex_enter.h>
#include <innodb/logger/info.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/wait/ut_wait_for.h>

#ifndef UNIV_HOTBACKUP

#include "log0log.h"
#include "log0log.h"
#include "log0recv.h"  /* recv_recovery_is_on() */
//#include "srv0start.h" /* SRV_SHUTDOWN_FLUSH_PHASE */



void log_free_check_wait(log_t &log, sn_t sn) {
  auto stop_condition = [&log, sn](bool) {

    const sn_t margins = log_free_check_margin(log);

    const lsn_t start_lsn = log_translate_sn_to_lsn(sn + margins);

    const lsn_t checkpoint_lsn = log.last_checkpoint_lsn.load();

    if (start_lsn <= checkpoint_lsn + log.lsn_capacity_for_free_check) {
      /* No reason to wait anymore. */
      return (true);
    }

    log_request_checkpoint(log, true,
                           start_lsn - log.lsn_capacity_for_free_check);

    return (false);
  };

  const auto wait_stats = ut_wait_for(0, 100, stop_condition);

  MONITOR_INC_WAIT_STATS(MONITOR_LOG_ON_FILE_SPACE_, wait_stats);
}



#endif /* !UNIV_HOTBACKUP */
