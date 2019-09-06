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

/** @file include/trx0trx.h
 The transaction

 Created 3/26/1996 Heikki Tuuri
 *******************************************************/

#ifndef trx0trx_h
#define trx0trx_h

#include <innodb/univ/univ.h>


#include <innodb/trx_trx/trx_is_rseg_updated.h>
#include <innodb/trx_trx/trx_is_rseg_assigned.h>
#include <innodb/trx_trx/trx_get_que_state_str.h>
#include <innodb/trx_trx/trx_get_id_for_print.h>
#include <innodb/trx_trx/trx_reference.h>
#include <innodb/trx_trx/trx_release_reference.h>
#include <innodb/trx_trx/trx_is_referenced.h>
#include <innodb/trx_trx/trx_arbitrate.h>
#include <innodb/trx_trx/trx_is_high_priority.h>
#include <innodb/trx_trx/trx_is_rseg_updated.h>
#include <innodb/trx_trx/trx_is_autocommit_non_locking.h>
#include <innodb/trx_trx/trx_is_ac_nl_ro.h>
#include <innodb/trx_trx/assert_trx_in_rw_list.h>
#include <innodb/trx_trx/check_trx_state.h>
#include <innodb/trx_trx/lock_pool_t.h>
#include <innodb/trx_trx/trx_mod_tables_t.h>
#include <innodb/trx_trx/trx_undo_ptr_t.h>
#include <innodb/trx_trx/trx_rsegs_t.h>
#include <innodb/trx_trx/TrxVersion.h>
#include <innodb/trx_trx/hit_list_t.h>
#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_mutex_own.h>
#include <innodb/trx_trx/trx_mutex_enter.h>
#include <innodb/trx_trx/trx_mutex_exit.h>

#include <list>
#include <set>

#include "ha_prototypes.h"


struct lock_t;
struct lock_sys_t;
struct lock_table_t;

#include "log0log.h"

#include "que0types.h"
#include "trx0xa.h"
#include "usr0types.h"
#include <innodb/vector/vector.h>
#ifndef UNIV_HOTBACKUP
#include "fts0fts.h"
#endif /* !UNIV_HOTBACKUP */
#include "srv0srv.h"

struct trx_undo_t;
struct trx_rseg_t;
struct trx_named_savept_t;
struct commit_node_t;

// Forward declaration
struct mtr_t;

// Forward declaration
class ReadView;

// Forward declaration
class FlushObserver;

#include <innodb/trx_trx/trx_dummy_sess.h>
#include <innodb/trx_trx/trx_erase_lists.h>
#include <innodb/trx_trx/flags.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/trx_trx/trx_set_flush_observer.h>
#include <innodb/trx_trx/trx_set_detailed_error.h>
#include <innodb/trx_trx/trx_set_detailed_error_from_file.h>
#include <innodb/trx_trx/trx_allocate_for_background.h>
#include <innodb/trx_trx/trx_allocate_for_mysql.h>
#include <innodb/trx_trx/trx_free_resurrected.h>
#include <innodb/trx_trx/trx_free_for_background.h>
#include <innodb/trx_trx/trx_free_prepared.h>
#include <innodb/trx_trx/trx_disconnect_plain.h>
#include <innodb/trx_trx/trx_disconnect_prepared.h>
#include <innodb/trx_trx/trx_free_for_mysql.h>
#include <innodb/trx_trx/trx_resurrect_locks.h>
#include <innodb/trx_trx/trx_lists_init_at_db_start.h>
#include <innodb/trx_trx/trx_start_if_not_started_xa_low.h>
#include <innodb/trx_trx/trx_start_if_not_started_low.h>
#include <innodb/trx_trx/trx_start_internal_low.h>
#include <innodb/trx_trx/trx_start_internal_read_only_low.h>
#include <innodb/trx_trx/trx_start_if_not_started_xa.h>
#include <innodb/trx_trx/trx_start_if_not_started.h>
#include <innodb/trx_trx/trx_start_internal.h>
#include <innodb/trx_trx/trx_start_internal_read_only.h>
#include <innodb/trx_trx/trx_commit.h>
#include <innodb/trx_trx/trx_commit_low.h>
#include <innodb/trx_trx/trx_cleanup_at_db_startup.h>
#include <innodb/trx_trx/trx_commit_for_mysql.h>
#include <innodb/trx_trx/trx_prepare_for_mysql.h>
#include <innodb/trx_trx/trx_recover_for_mysql.h>
#include <innodb/trx_trx/trx_get_trx_by_xid.h>
#include <innodb/trx_trx/trx_commit_complete_for_mysql.h>
#include <innodb/trx_trx/trx_mark_sql_stat_end.h>
#include <innodb/trx_trx/trx_assign_read_view.h>
#include <innodb/trx_trx/trx_commit_or_rollback_prepare.h>
#include <innodb/trx_trx/trx_commit_node_create.h>
#include <innodb/trx_trx/trx_commit_step.h>
#include <innodb/trx_trx/trx_print_low.h>
#include <innodb/trx_trx/trx_print_latched.h>
#include <innodb/trx_trx/trx_print.h>
#include <innodb/trx_trx/trx_assert_started.h>
#include <innodb/trx_trx/trx_is_interrupted.h>
#include <innodb/trx_trx/trx_is_strict.h>
#include <innodb/trx_trx/TRX_WEIGHT.h>
#include <innodb/trx_trx/trx_weight_ge.h>
#include <innodb/trx_trx/trx_assign_rseg_temp.h>
#include <innodb/trx_trx/trx_pool_init.h>
#include <innodb/trx_trx/trx_pool_close.h>
#include <innodb/trx_trx/trx_set_rw_mode.h>
#include <innodb/trx_trx/trx_kill_blocking.h>
#include <innodb/trx_trx/trx_immutable_id.h>
#include <innodb/trx_trx/trx_lock_wait_timeout_get.h>
#include <innodb/trx_trx/assert_trx_is_free.h>
#include <innodb/trx_trx/assert_trx_is_inactive.h>
#include <innodb/trx_trx/assert_trx_nonlocking_or_in_list.h>


#endif /* !UNIV_HOTBACKUP */







#include <innodb/trx_trx/trx_rseg_type_t.h>





#ifndef UNIV_HOTBACKUP


#include <innodb/trx_trx/trx_isolation_level.h>
#include <innodb/trx_trx/trx_is_started.h>
#include <innodb/trx_trx/commit_node_state.h>





/** Commit command node in a query graph */
struct commit_node_t {
  que_common_t common;          /*!< node type: QUE_NODE_COMMIT */
  enum commit_node_state state; /*!< node execution state */
};




#include <innodb/trx_trx/TrxInInnoDB.h>




#endif /* !UNIV_HOTBACKUP */

#endif
