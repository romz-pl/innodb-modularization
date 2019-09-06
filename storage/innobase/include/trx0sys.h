/*****************************************************************************

Copyright (c) 1996, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/trx0sys.h
 Transaction system

 Created 3/26/1996 Heikki Tuuri
 *******************************************************/

#pragma once

#include <innodb/univ/univ.h>



#include "buf0buf.h"
#include "fil0fil.h"
#include "mtr0log.h"
#include "srv0srv.h"
#include <atomic>
#include <innodb/buf_block/buf_block_get_frame.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/lst/lst.h>
#include <innodb/mtr/mtr_read_ulint.h>
#include <innodb/page/header.h>
#include <innodb/tablespace/Space_Ids.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/trx_sys_undo_spaces.h>
#include <innodb/trx_sys/flags.h>
#include <innodb/trx_sys/trx_assert_recovered.h>
#include <innodb/trx_sys/trx_get_rw_trx_by_id.h>
#include <innodb/trx_sys/trx_read_trx_id.h>
#include <innodb/trx_sys/trx_rseg_n_slots_debug.h>
#include <innodb/trx_sys/trx_rw_is_active.h>
#include <innodb/trx_sys/trx_rw_is_active_low.h>
#include <innodb/trx_sys/trx_rw_min_trx_id.h>
#include <innodb/trx_sys/trx_sys.h>
#include <innodb/trx_sys/trx_sys_any_active_transactions.h>
#include <innodb/trx_sys/trx_sys_close.h>
#include <innodb/trx_sys/trx_sys_create.h>
#include <innodb/trx_sys/trx_sys_create_sys_pages.h>
#include <innodb/trx_sys/trx_sys_flush_max_trx_id.h>
#include <innodb/trx_sys/trx_sys_get_max_trx_id.h>
#include <innodb/trx_sys/trx_sys_get_new_trx_id.h>
#include <innodb/trx_sys/trx_sys_hdr_page.h>
#include <innodb/trx_sys/trx_sys_init_at_db_start.h>
#include <innodb/trx_sys/trx_sys_mutex_enter.h>
#include <innodb/trx_sys/trx_sys_mutex_exit.h>
#include <innodb/trx_sys/trx_sys_mutex_own.h>
#include <innodb/trx_sys/trx_sys_need_rollback.h>
#include <innodb/trx_sys/trx_sys_print_mysql_binlog_offset.h>
#include <innodb/trx_sys/trx_sys_print_mysql_binlog_offset_from_page.h>
#include <innodb/trx_sys/trx_sys_rw_trx_add.h>
#include <innodb/trx_sys/trx_sys_t.h>


