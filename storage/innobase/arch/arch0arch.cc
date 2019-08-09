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

/** @file arch/arch0arch.cc
 Common implementation for redo log and dirty page archiver system

 *******************************************************/

#include <innodb/io/os_file_create.h>
#include <innodb/allocator/ut_malloc.h>
#include <innodb/logger/info.h>
#include <innodb/sync_event/os_event_destroy.h>
#include <innodb/sync_event/os_event_wait.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/io/pfs.h>
#include <innodb/io/os_file_delete.h>
#include <innodb/io/os_file_copy.h>
#include <innodb/io/os_file_read.h>
#include <innodb/io/os_file_write.h>
#include <innodb/io/os_file_status.h>
#include <innodb/io/os_file_seek.h>
#include <innodb/io/os_file_scan_directory.h>
#include <innodb/io/os_file_create_directory.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/IORequest.h>
#include <innodb/formatting/formatting.h>
#include <innodb/log_arch/log_archiver_thread_event.h>

#include "arch0arch.h"
#include "os0thread-create.h"
#include "srv0srv.h"

int start_page_archiver_background() {
  bool ret;
  char errbuf[MYSYS_STRERROR_SIZE];

  ret = os_file_create_directory(ARCH_DIR, false);

  if (ret) {
    page_archiver_is_active = true;

    os_thread_create(page_archiver_thread_key, page_archiver_thread);
  } else {
    my_error(ER_CANT_CREATE_FILE, MYF(0), ARCH_DIR, errno,
             my_strerror(errbuf, sizeof(errbuf), errno));

    return (ER_CANT_CREATE_FILE);
  }

  return (0);
}

