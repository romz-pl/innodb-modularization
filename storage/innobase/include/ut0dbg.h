/*****************************************************************************

Copyright (c) 1994, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/ut0dbg.h
 Debug utilities for Innobase

 Created 1/30/1994 Heikki Tuuri
 **********************************************************************/

#ifndef ut0dbg_h
#define ut0dbg_h

/* Do not include univ.i because univ.i includes this. */
// #include <innodb/univ/univ.h>

#include <innodb/assert/assert.h>
#include <innodb/compiler_hints/compiler_hints.h>


/** Abort execution. */
#define ut_error ut_dbg_assertion_failed(0, __FILE__, (ulint)__LINE__)


#endif
