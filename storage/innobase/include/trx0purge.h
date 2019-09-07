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

/** @file include/trx0purge.h
 Purge old versions

 Created 3/26/1996 Heikki Tuuri
 *******************************************************/

#ifndef trx0purge_h
#define trx0purge_h

#include <innodb/univ/univ.h>



#include "fil0fil.h"
#include "page0page.h"
#include "que0types.h"
#include "usr0sess.h"

#include <innodb/allocator/UT_NEW_NOKEY.h>
#include <innodb/read/ReadView.h>
#include <innodb/tablespace/Space_Ids.h>
#include <innodb/trx_purge/Tablespace.h>
#include <innodb/trx_purge/Tablespaces.h>
#include <innodb/trx_purge/Truncate.h>
#include <innodb/trx_purge/TrxUndoRsegsIterator.h>
#include <innodb/trx_purge/add_space_to_construction_list.h>
#include <innodb/trx_purge/clear_construction_list.h>
#include <innodb/trx_purge/ddl_mutex.h>
#include <innodb/trx_purge/done_logging.h>
#include <innodb/trx_purge/flags.h>
#include <innodb/trx_purge/get_next_available_space_num.h>
#include <innodb/trx_purge/id2next_id.h>
#include <innodb/trx_purge/id2num.h>
#include <innodb/trx_purge/init_space_id_bank.h>
#include <innodb/trx_purge/is_active.h>
#include <innodb/trx_purge/is_active_truncate_log_present.h>
#include <innodb/trx_purge/is_reserved.h>
#include <innodb/trx_purge/is_under_construction.h>
#include <innodb/trx_purge/make_file_name.h>
#include <innodb/trx_purge/make_space_name.h>
#include <innodb/trx_purge/next_space_id.h>
#include <innodb/trx_purge/num2id.h>
#include <innodb/trx_purge/purge_iter_t.h>
#include <innodb/trx_purge/purge_state_t.h>
#include <innodb/trx_purge/purge_sys.h>
#include <innodb/trx_purge/s_under_construction.h>
#include <innodb/trx_purge/set_active.h>
#include <innodb/trx_purge/space_id_account.h>
#include <innodb/trx_purge/space_id_bank.h>
#include <innodb/trx_purge/spaces.h>
#include <innodb/trx_purge/start_logging.h>
#include <innodb/trx_purge/trx_purge.h>
#include <innodb/trx_purge/trx_purge_add_update_undo_to_history.h>
#include <innodb/trx_purge/trx_purge_get_log_from_hist.h>
#include <innodb/trx_purge/trx_purge_run.h>
#include <innodb/trx_purge/trx_purge_state.h>
#include <innodb/trx_purge/trx_purge_stop.h>
#include <innodb/trx_purge/trx_purge_sys_close.h>
#include <innodb/trx_purge/trx_purge_sys_create.h>
#include <innodb/trx_purge/trx_purge_t.h>
#include <innodb/trx_purge/unuse_space_id.h>
#include <innodb/trx_purge/use_next_space_id.h>
#include <innodb/trx_purge/use_space_id.h>
#include <innodb/trx_types/PQMutex.h>
#include <innodb/trx_types/Rsegs.h>
#include <innodb/trx_types/TrxUndoRsegs.h>
#include <innodb/trx_types/purge_pq_t.h>
#include <innodb/trx_types/trx_rseg_t.h>
#include <innodb/trx_undo/flags.h>

struct trx_purge_t;
struct trx_undo_ptr_t;



#endif /* trx0purge_h */
