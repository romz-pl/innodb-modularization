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








#include "que0types.h"




#ifndef UNIV_HOTBACKUP

#include <innodb/trx_trx/commit_node_state.h>

/** Commit command node in a query graph */
struct commit_node_t {
  que_common_t common;          /*!< node type: QUE_NODE_COMMIT */
  enum commit_node_state state; /*!< node execution state */
};


#endif /* !UNIV_HOTBACKUP */

#endif
