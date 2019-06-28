/*****************************************************************************

Copyright (c) 1995, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/sync0types.h
 Global types for sync

 Created 9/5/1995 Heikki Tuuri
 *******************************************************/

#ifndef sync0types_h
#define sync0types_h


#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG
/** Set when InnoDB has invoked exit(). */
extern bool innodb_calling_exit;
#endif /* UNIV_DEBUG */


/** The new (C++11) syntax allows the following and we should use it when it
is available on platforms that we support.

        enum class mutex_state_t : lock_word_t { ... };
*/

/** Mutex states. */
enum mutex_state_t {
  /** Mutex is free */
  MUTEX_STATE_UNLOCKED = 0,

  /** Mutex is acquired by some thread. */
  MUTEX_STATE_LOCKED = 1,

  /** Mutex is contended and there are threads waiting on the lock. */
  MUTEX_STATE_WAITERS = 2
};




#ifndef UNIV_LIBRARY


/** Register a latch, called when it is created
@param[in]	ptr		Latch instance that was created
@param[in]	filename	Filename where it was created
@param[in]	line		Line number in filename */
void sync_file_created_register(const void *ptr, const char *filename,
                                uint16_t line);

/** Deregister a latch, called when it is destroyed
@param[in]	ptr		Latch to be destroyed */
void sync_file_created_deregister(const void *ptr);

/** Get the string where the file was created. Its format is "name:line"
@param[in]	ptr		Latch instance
@return created information or "" if can't be found */
std::string sync_file_created_get(const void *ptr);

#endif /* !UNIV_LIBRARY */


#endif /* sync0types_h */
