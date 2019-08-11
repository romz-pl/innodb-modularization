/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2009, Google Inc.

Portions of this file contain modifications contributed and copyrighted by
Google, Inc. Those modifications are gratefully acknowledged and are described
briefly in the InnoDB documentation. The contributions by Google are
incorporated with their permission, and subject to the conditions contained in
the file COPYING.Google.

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

/**************************************************/ /**
 @file log/log0log.cc


 Created 12/9/1995 Heikki Tuuri
 *******************************************************/

#include <innodb/allocator/aligned_pointer.h>
#include <innodb/logger/info.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_types/log_checksum_func_t.h>
#include <innodb/log_sn/LSN_PF.h>
#include <innodb/log_types/log_header_format_t.h>
#include <innodb/log_redo/log_allocate_write_events.h>
#include <innodb/log_redo/log_deallocate_write_events.h>
#include <innodb/log_write/log_allocate_recent_written.h>
#include <innodb/log_write/log_deallocate_recent_written.h>
#include <innodb/log_closer/log_allocate_recent_closed.h>
#include <innodb/log_closer/log_deallocate_recent_closed.h>
#include <innodb/log_files/log_allocate_file_header_buffers.h>
#include <innodb/log_files/log_deallocate_file_header_buffers.h>
#include <innodb/log_buffer/log_calc_buf_size.h>
#include <innodb/log_sys/log_sys_object.h>
#include <innodb/log_redo/pfs.h>
#include <innodb/log_closer/log_closer.h>
#include <innodb/log_flush/log_flush_notifier.h>
#include <innodb/log_write/log_write_notifier.h>

#ifndef UNIV_HOTBACKUP

#include <debug_sync.h>
#include <sys/types.h>
#include <time.h>
#include "dict0boot.h"
#include "ha_prototypes.h"
#include <innodb/thread/os_thread_create.h>
#include "trx0sys.h"




/**************************************************/ /**

 @name	Start / stop of background threads

 *******************************************************/

/* @{ */


void log_start_background_threads(log_t &log) {
  ib::info(ER_IB_MSG_1258) << "Log background threads are being started...";

  std::atomic_thread_fence(std::memory_order_seq_cst);

  log_background_threads_inactive_validate(log);

  ut_ad(!log.disable_redo_writes);
  ut_a(!srv_read_only_mode);
  ut_a(log.sn.load() > 0);

  log.closer_thread_alive.store(true);
  log.checkpointer_thread_alive.store(true);
  log.writer_thread_alive.store(true);
  log.flusher_thread_alive.store(true);
  log.write_notifier_thread_alive.store(true);
  log.flush_notifier_thread_alive.store(true);

  log.should_stop_threads.store(false);

  std::atomic_thread_fence(std::memory_order_seq_cst);

  os_thread_create(log_checkpointer_thread_key, log_checkpointer, &log);

  os_thread_create(log_closer_thread_key, log_closer, &log);

  os_thread_create(log_writer_thread_key, log_writer, &log);

  os_thread_create(log_flusher_thread_key, log_flusher, &log);

  os_thread_create(log_write_notifier_thread_key, log_write_notifier, &log);

  os_thread_create(log_flush_notifier_thread_key, log_flush_notifier, &log);

  log_background_threads_active_validate(log);
}


/* @} */




#endif /* !UNIV_HOTBACKUP */
