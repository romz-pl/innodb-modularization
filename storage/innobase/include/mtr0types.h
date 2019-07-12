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

/** @file include/mtr0types.h
 Mini-transaction buffer global types

 Created 11/26/1995 Heikki Tuuri
 *******************************************************/

#ifndef mtr0types_h
#define mtr0types_h

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_type_t.h>
#include <innodb/error/ut_error.h>

#include <innodb/machine/data.h>
#include <innodb/tablespace/mlog_id_t.h>
#include <innodb/tablespace/mach_read_ulint.h>


struct mtr_t;

/** Logging modes for a mini-transaction */
enum mtr_log_t {
  /** Default mode: log all operations modifying disk-based data */
  MTR_LOG_ALL = 21,

  /** Log no operations and dirty pages are not added to the flush list */
  MTR_LOG_NONE = 22,

  /** Don't generate REDO log but add dirty pages to flush list */
  MTR_LOG_NO_REDO = 23,

  /** Inserts are logged in a shorter form */
  MTR_LOG_SHORT_INSERTS = 24
};


/* @} */




/** Types for the mlock objects to store in the mtr memo; NOTE that the
first 3 values must be RW_S_LATCH, RW_X_LATCH, RW_NO_LATCH */
enum mtr_memo_type_t {
  MTR_MEMO_PAGE_S_FIX = RW_S_LATCH,

  MTR_MEMO_PAGE_X_FIX = RW_X_LATCH,

  MTR_MEMO_PAGE_SX_FIX = RW_SX_LATCH,

  MTR_MEMO_BUF_FIX = RW_NO_LATCH,

#ifdef UNIV_DEBUG
  MTR_MEMO_MODIFY = 32,
#endif /* UNIV_DEBUG */

  MTR_MEMO_S_LOCK = 64,

  MTR_MEMO_X_LOCK = 128,

  MTR_MEMO_SX_LOCK = 256
};

#ifdef UNIV_DEBUG
#define MTR_MAGIC_N 54551
#endif /* UNIV_DEBUG */

enum mtr_state_t {
  MTR_STATE_INIT = 0,
  MTR_STATE_ACTIVE = 12231,
  MTR_STATE_COMMITTING = 56456,
  MTR_STATE_COMMITTED = 34676
};

#endif /* mtr0types_h */
