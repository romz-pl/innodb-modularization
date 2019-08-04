/*****************************************************************************

Copyright (c) 1997, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file ha/hash0hash.cc
 The simple hash table utility

 Created 5/20/1997 Heikki Tuuri
 *******************************************************/

#include <innodb/allocator/ut_free.h>
#include <innodb/math/ut_is_2pow.h>
#include <innodb/sync_latch/sync_latch_get_level.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_rw/rw_lock_create.h>
#include <innodb/sync_os/pfs.h>

#include "hash0hash.h"
#include "mem0mem.h"







