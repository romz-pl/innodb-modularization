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

/** @file include/trx0rseg.h
 Rollback segment

 Created 3/26/1996 Heikki Tuuri
 *******************************************************/

#ifndef trx0rseg_h
#define trx0rseg_h

#include <innodb/univ/univ.h>

#include <innodb/tablespace/Space_Ids.h>
#include <innodb/tablespace/srv_tmp_space.h>
#include <innodb/trx_rseq/flags.h>
#include <innodb/trx_rseq/trx_rseg_adjust_rollback_segments.h>
#include <innodb/trx_rseq/trx_rseg_array_create.h>
#include <innodb/trx_rseq/trx_rseg_create.h>
#include <innodb/trx_rseq/trx_rseg_get_n_undo_tablespaces.h>
#include <innodb/trx_rseq/trx_rseg_header_create.h>
#include <innodb/trx_rseq/trx_rseg_init_rollback_segments.h>
#include <innodb/trx_rseq/trx_rseg_mem_create.h>
#include <innodb/trx_rseq/trx_rseg_upgrade_undo_tablespaces.h>
#include <innodb/trx_rseq/trx_rsegf_get.h>
#include <innodb/trx_rseq/trx_rsegf_get_new.h>
#include <innodb/trx_rseq/trx_rsegf_get_nth_undo.h>
#include <innodb/trx_rseq/trx_rsegf_set_nth_undo.h>
#include <innodb/trx_rseq/trx_rsegf_undo_find_free.h>
#include <innodb/trx_rseq/trx_rsegs_init.h>
#include <innodb/trx_rseq/trx_rsegsf_set_page_no.h>
#include <innodb/trx_sys/trx_sys.h>
#include <innodb/trx_types/purge_pq_t.h>
#include <innodb/trx_types/trx_rseg_t.h>
#include <innodb/trx_types/trx_rsegf_t.h>
#include <innodb/trx_types/trx_rsegsf_t.h>

#include "mtr0log.h"
#include "srv0srv.h"
#include "trx0purge.h"

#include <vector>
#include "fut0lst.h"





#endif
