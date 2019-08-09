/*****************************************************************************

Copyright (c) 2017, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/arch0arch.h
 Common interface for redo log and dirty page archiver system

 *******************************************************/

#ifndef ARCH_ARCH_INCLUDE
#define ARCH_ARCH_INCLUDE

#include <innodb/univ/univ.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/io/os_file_flush.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_get_size.h>
#include <innodb/error/dberr_t.h>
#include <innodb/log_arch/flags.h>
#include <innodb/log_arch/log_archiver_thread_event.h>
#include <innodb/log_arch/log_archiver_is_active.h>
#include <innodb/log_arch/Arch_Client_State.h>
#include <innodb/log_arch/Arch_State.h>
#include <innodb/log_arch/Arch_Blk_State.h>
#include <innodb/log_arch/Arch_Blk_Type.h>
#include <innodb/log_arch/Arch_Blk_Flush_Type.h>
#include <innodb/log_arch/Arch_Page_Dblwr_Offset.h>
#include <innodb/log_arch/Arch_Page_Pos.h>
#include <innodb/log_arch/Arch_Point.h>
#include <innodb/log_arch/Arch_Reset_File.h>
#include <innodb/log_arch/Arch_Reset.h>
#include <innodb/log_arch/Arch_Block.h>
#include <innodb/log_arch/Arch_File_Ctx.h>
#include <innodb/log_arch/Arch_Group.h>
#include <innodb/log_arch/Arch_Grp_List.h>
#include <innodb/log_arch/Arch_Grp_List_Iter.h>
#include <innodb/log_arch/Arch_Log_Sys.h>
#include <innodb/log_arch/Arch_Block_Vec.h>
#include <innodb/log_arch/ArchPageData.h>
#include <innodb/log_arch/Arch_Page_Sys.h>
#include <innodb/log_arch/arch_log_sys.h>
#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/log_arch/arch_remove_file.h>
#include <innodb/log_arch/arch_remove_dir.h>
#include <innodb/log_arch/page_archiver_thread_event.h>
#include <innodb/log_arch/arch_init.h>
#include <innodb/sync_event/os_event_destroy.h>
#include <innodb/log_arch/log_archiver_thread.h>
#include <innodb/log_arch/page_archiver_thread.h>

#include <mysql/components/services/page_track_service.h>
#include "log0log.h"

#include <list>
#include <deque>

class buf_page_t;

#include <innodb/error/dberr_t.h>
#include <innodb/sync_event/os_event_t.h>
#include <innodb/log_arch/page_archiver_is_active.h>
#include <innodb/log_arch/start_page_archiver_background.h>
#include <innodb/log_arch/start_log_archiver_background.h>


#endif /* ARCH_ARCH_INCLUDE */
