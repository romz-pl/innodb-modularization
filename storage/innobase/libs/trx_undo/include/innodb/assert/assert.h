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

#pragma once

#include <innodb/univ/univ.h>
#include <innodb/compiler_hints/compiler_hints.h>

/** Report a failed assertion. */
[[noreturn]] void ut_dbg_assertion_failed(
    const char *expr, /*!< in: the failed assertion */
    const char *file, /*!< in: source file containing the assertion */
    ulint line);      /*!< in: line number of the assertion */

/** Abort execution if EXPR does not evaluate to nonzero.
@param EXPR assertion expression that should hold */
#define ut_a(EXPR)                                               \
  do {                                                           \
    if (UNIV_UNLIKELY(!(ulint)(EXPR))) {                         \
      ut_dbg_assertion_failed(#EXPR, __FILE__, (ulint)__LINE__); \
    }                                                            \
  } while (0)


#ifdef UNIV_DEBUG
/** Debug assertion. Does nothing unless UNIV_DEBUG is defined. */
#define ut_ad(EXPR) ut_a(EXPR)
/** Debug statement. Does nothing unless UNIV_DEBUG is defined. */
#define ut_d(EXPR) EXPR
#else
/** Debug assertion. Does nothing unless UNIV_DEBUG is defined. */
#define ut_ad(EXPR)
/** Debug statement. Does nothing unless UNIV_DEBUG is defined. */
#define ut_d(EXPR)
#endif

/** Debug crash point */
#ifdef UNIV_DEBUG
#define DBUG_INJECT_CRASH(prefix, count)            \
  do {                                              \
    char buf[64];                                   \
    snprintf(buf, sizeof buf, prefix "_%u", count); \
    DBUG_EXECUTE_IF(buf, DBUG_SUICIDE(););          \
  } while (0)

#define DBUG_INJECT_CRASH_WITH_LOG_FLUSH(prefix, count)                \
  do {                                                                 \
    char buf[64];                                                      \
    snprintf(buf, sizeof buf, prefix "_%u", count);                    \
    DBUG_EXECUTE_IF(buf, log_buffer_flush_to_disk(); DBUG_SUICIDE();); \
  } while (0)
#else
#define DBUG_INJECT_CRASH(prefix, count)
#define DBUG_INJECT_CRASH_WITH_LOG_FLUSH(prefix, count)
#endif

/** Silence warnings about an unused variable by doing a null assignment.
@param A the unused variable */
#define UT_NOT_USED(A) A = A




