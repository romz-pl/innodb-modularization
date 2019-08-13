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

/** @file include/dict0types.h
 Data dictionary global types

 Created 1/8/1996 Heikki Tuuri
 *******************************************************/

#ifndef dict0types_h
#define dict0types_h

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>
#include <innodb/record/flag.h>




#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/dict/flags.h>

#include "ibuf0types.h" /* IBUF_SPACE_ID */


struct dict_sys_t;
struct dict_col_t;
struct dict_field_t;
struct dict_index_t;
struct dict_table_t;
struct dict_foreign_t;
struct dict_v_col_t;

struct ind_node_t;
struct tab_node_t;
struct dict_add_v_col_t;















#if defined UNIV_DEBUG || defined UNIV_IBUF_DEBUG
/** Flag to control insert buffer debugging. */
extern uint ibuf_debug;
#endif /* UNIV_DEBUG || UNIV_IBUF_DEBUG */




#endif
