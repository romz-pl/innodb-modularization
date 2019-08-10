/*****************************************************************************

Copyright (c) 2017, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file clone/clone0copy.cc
 Innodb copy snapshot data

 *******************************************************/


#include <innodb/io/os_file_status.h>
#include <innodb/io/os_file_size_t.h>
#include <innodb/io/os_file_get_size.h>
#include <innodb/clone/Clone_Snapshot.h>
#include <innodb/tablespace/consts.h>
#include <innodb/io/macros.h>

#include "buf0dump.h"


int Clone_Snapshot::add_buf_pool_file() {
  os_file_type_t type;
  os_file_size_t file_size;

  ib_uint64_t size_bytes;
  char path[OS_FILE_MAX_PATH];
  bool exists = false;

  /* Generate the file name. */
  buf_dump_generate_path(path, sizeof(path));

  os_file_status(path, &exists, &type);

  /* Add if the file is found. */
  int err = 0;

  if (exists) {
    file_size = os_file_get_size(path);
    size_bytes = file_size.m_total_size;

    /* Always the first file in list */
    ut_ad(m_num_data_files == 0);

    err = add_file(path, size_bytes, dict_sys_t_s_invalid_space_id, true);
  }

  return (err);
}

