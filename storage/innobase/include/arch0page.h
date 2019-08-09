/*****************************************************************************

Copyright (c) 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/arch0page.h
 Innodb interface for modified page archive

 *******************************************************/

#ifndef ARCH_PAGE_INCLUDE
#define ARCH_PAGE_INCLUDE

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/log_arch/flags.h>
#include <innodb/log_arch/Arch_Client_State.h>
#include <innodb/log_arch/Arch_Page_Pos.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/log_arch/Page_Arch_Cbk.h>
#include <innodb/log_arch/Page_Wait_Flush_Archiver_Cbk.h>
#include <innodb/log_arch/Page_Arch_Cbk.h>
#include <innodb/log_arch/Page_Arch_Client_Ctx.h>

#include "arch0arch.h"
#include "buf0buf.h"








#endif /* ARCH_PAGE_INCLUDE */
