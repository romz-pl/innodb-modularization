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

/** @file include/sync0rw.h
 The read-write lock (for threads, not for database transactions)

 Created 9/11/1995 Heikki Tuuri
 *******************************************************/

#ifndef sync0rw_h
#define sync0rw_h

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_t.h>
#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/counter/counter.h>
#include <innodb/lst/lst.h>

#include <innodb/sync_rw/rw_lock_list_t.h>
#include <innodb/sync_rw/rw_lock_stats_t.h>

struct rw_lock_t;








extern rw_lock_list_t rw_lock_list;
extern ib_mutex_t rw_lock_list_mutex;

#ifndef UNIV_HOTBACKUP
/** Counters for RW locks. */
extern rw_lock_stats_t rw_lock_stats;
#endif /* !UNIV_HOTBACKUP */



#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP


#include "sync0rw.ic"

#endif /* !UNIV_HOTBACKUP */
#endif /* !UNIV_LIBRARY */


#endif /* sync0rw.h */
