/*****************************************************************************

Copyright (c) 2013, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file fsp/fsp0space.cc
 General shared tablespace implementation.

 Created 2012-11-16 by Sunny Bains as srv/srv0space.cc
 *******************************************************/

#include <innodb/logger/info.h>
#include <innodb/io/pfs.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/tablespace/Tablespace.h>
#include <innodb/tablespace/srv_tmp_space.h>
#include <innodb/tablespace/srv_sys_space.h>

#include "ha_prototypes.h"
#include "fsp0fsp.h"
#include "my_sys.h"


/** Open or Create the data files if they do not exist.
@param[in]	is_temp	whether this is a temporary tablespace
@return DB_SUCCESS or error code */
dberr_t Tablespace::open_or_create(bool is_temp) {
  fil_space_t *space = NULL;
  dberr_t err = DB_SUCCESS;

  ut_ad(!m_files.empty());

  files_t::iterator begin = m_files.begin();
  files_t::iterator end = m_files.end();

  for (files_t::iterator it = begin; it != end; ++it) {
    if (it->m_exists) {
      err = it->open_or_create(m_ignore_read_only ? false : srv_read_only_mode);
    } else {
      err = it->open_or_create(m_ignore_read_only ? false : srv_read_only_mode);

      /* Set the correct open flags now that we have
      successfully created the file. */
      if (err == DB_SUCCESS) {
        file_found(*it);
      }
    }

    if (err != DB_SUCCESS) {
      break;
    }

    bool atomic_write;

#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)
    if (!srv_use_doublewrite_buf) {
      atomic_write = fil_fusionio_enable_atomic_write(it->m_handle);
    } else {
      atomic_write = false;
    }
#else
    atomic_write = false;
#endif /* !NO_FALLOCATE && UNIV_LINUX */

    /* We can close the handle now and open the tablespace
    the proper way. */
    it->close();

    if (it == begin) {
      /* First data file. */

      uint32_t flags = fsp_flags_set_page_size(0, univ_page_size);

      /* Create the tablespace entry for the multi-file
      tablespace in the tablespace manager. */
      space =
          fil_space_create(m_name, m_space_id, flags,
                           is_temp ? FIL_TYPE_TEMPORARY : FIL_TYPE_TABLESPACE);
    }

    ut_ad(fil_validate());

    /* Create the tablespace node entry for this data file. */
    if (!fil_node_create(it->m_filepath, it->m_size, space, false,
                         atomic_write)) {
      err = DB_ERROR;
      break;
    }
  }

  return (err);
}








