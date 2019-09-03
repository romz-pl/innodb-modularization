/*****************************************************************************

Copyright (c) 1996, 2018, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2012, Facebook Inc.

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

/** @file dict/dict0mem.cc
 Data dictionary memory object creation

 Created 1/8/1996 Heikki Tuuri
 ***********************************************************************/


#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_is_online_ddl.h>
#include <innodb/read/ReadView.h>

#include "trx0trx.h"



#ifndef UNIV_HOTBACKUP


/** Check whether index can be used by transaction
@param[in] trx		transaction*/
bool dict_index_t::is_usable(const trx_t *trx) const {
  /* Indexes that are being created are not usable. */
  if (!is_clustered() && dict_index_is_online_ddl(this)) {
    return false;
  }

  /* Cannot use a corrupted index. */
  if (is_corrupted()) {
    return false;
  }

  /* Check if the specified transaction can see this index. */
  return (table->is_temporary() || trx_id == 0 ||
          !MVCC::is_view_active(trx->read_view) ||
          trx->read_view->changes_visible(trx_id, table->name));
}
#endif /* !UNIV_HOTBACKUP */
