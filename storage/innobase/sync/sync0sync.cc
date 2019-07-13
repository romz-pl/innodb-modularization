/*****************************************************************************

Copyright (c) 1995, 2018, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2008, Google Inc.

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

/** @file sync/sync0sync.cc
 Mutex, the basic synchronization primitive

 Created 9/5/1995 Heikki Tuuri
 *******************************************************/

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/MutexMonitor.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_rw/rw_lock_stats.h>
#include <innodb/sync_rw/rw_lock_list_mutex.h>
#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/rw_lock_list.h>
#include <innodb/sync_array/sync_array_print.h>

#include "sync0sync.h"

#include <innodb/formatting/formatting.h>






/** For monitoring active mutexes */
MutexMonitor *mutex_monitor;

/**
Prints wait info of the sync system.
@param file - where to print */
static void sync_print_wait_info(FILE *file) {
  fprintf(file,
          "RW-shared spins " UINT64PF ", rounds " UINT64PF
          ","
          " OS waits " UINT64PF
          "\n"
          "RW-excl spins " UINT64PF ", rounds " UINT64PF
          ","
          " OS waits " UINT64PF
          "\n"
          "RW-sx spins " UINT64PF ", rounds " UINT64PF
          ","
          " OS waits " UINT64PF "\n",
          (uint64_t)rw_lock_stats.rw_s_spin_wait_count,
          (uint64_t)rw_lock_stats.rw_s_spin_round_count,
          (uint64_t)rw_lock_stats.rw_s_os_wait_count,
          (uint64_t)rw_lock_stats.rw_x_spin_wait_count,
          (uint64_t)rw_lock_stats.rw_x_spin_round_count,
          (uint64_t)rw_lock_stats.rw_x_os_wait_count,
          (uint64_t)rw_lock_stats.rw_sx_spin_wait_count,
          (uint64_t)rw_lock_stats.rw_sx_spin_round_count,
          (uint64_t)rw_lock_stats.rw_sx_os_wait_count);

  fprintf(
      file,
      "Spin rounds per wait: %.2f RW-shared,"
      " %.2f RW-excl, %.2f RW-sx\n",
      (double)rw_lock_stats.rw_s_spin_round_count /
          std::max(uint64_t(1), (uint64_t)rw_lock_stats.rw_s_spin_wait_count),
      (double)rw_lock_stats.rw_x_spin_round_count /
          std::max(uint64_t(1), (uint64_t)rw_lock_stats.rw_x_spin_wait_count),
      (double)rw_lock_stats.rw_sx_spin_round_count /
          std::max(uint64_t(1), (uint64_t)rw_lock_stats.rw_sx_spin_wait_count));
}

/**
Prints info of the sync system.
@param file - where to print */
void sync_print(FILE *file) {
#ifdef UNIV_DEBUG
  rw_lock_list_print_info(file);
#endif /* UNIV_DEBUG */

  sync_array_print(file);

  sync_print_wait_info(file);
}



/** Enable the mutex monitoring */
void MutexMonitor::enable() {
  /** Note: We don't add any latch meta-data after startup. Therefore
  there is no need to use a mutex here. */

  LatchMetaData::iterator end = latch_meta.end();

  for (LatchMetaData::iterator it = latch_meta.begin(); it != end; ++it) {
    if (*it != NULL) {
      (*it)->get_counter()->enable();
    }
  }
}

/** Disable the mutex monitoring */
void MutexMonitor::disable() {
  /** Note: We don't add any latch meta-data after startup. Therefore
  there is no need to use a mutex here. */

  LatchMetaData::iterator end = latch_meta.end();

  for (LatchMetaData::iterator it = latch_meta.begin(); it != end; ++it) {
    if (*it != NULL) {
      (*it)->get_counter()->disable();
    }
  }
}

/** Reset the mutex monitoring counters */
void MutexMonitor::reset() {
  /** Note: We don't add any latch meta-data after startup. Therefore
  there is no need to use a mutex here. */

  LatchMetaData::iterator end = latch_meta.end();

  for (LatchMetaData::iterator it = latch_meta.begin(); it != end; ++it) {
    if (*it != NULL) {
      (*it)->get_counter()->reset();
    }
  }

  mutex_enter(&rw_lock_list_mutex);

  for (rw_lock_t *rw_lock = UT_LIST_GET_FIRST(rw_lock_list); rw_lock != NULL;
       rw_lock = UT_LIST_GET_NEXT(list, rw_lock)) {
    rw_lock->count_os_wait = 0;
  }

  mutex_exit(&rw_lock_list_mutex);
}
