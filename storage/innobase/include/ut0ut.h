/*****************************************************************************

Copyright (c) 1994, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/ut0ut.h
 Various utilities

 Created 1/20/1994 Heikki Tuuri
 ***********************************************************************/

/**************************************************/ /**
 @page PAGE_INNODB_UTILS Innodb utils

 Useful data structures:
 - @ref Link_buf - to track concurrent operations
 - @ref Sharded_rw_lock - sharded rw-lock (very fast s-lock, slow x-lock)

 *******************************************************/

#ifndef ut0ut_h
#define ut0ut_h

/* Do not include univ.i because univ.i includes this. */
// #include <innodb/univ/univ.h>


#include <string.h>
#include <algorithm>
#include <iterator>
#include <ostream>
#include <sstream>
#include <type_traits>
#include <innodb/time/ib_time_t.h>


#ifndef UNIV_HOTBACKUP
#include "os0atomic.h"
#endif /* !UNIV_HOTBACKUP */

#include <time.h>

#include <ctype.h>

#include <stdarg.h>
#include "ut/ut.h"
#include "ut0dbg.h"

#ifndef UNIV_NO_ERR_MSGS
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/derror.h"
#endif /* !UNIV_NO_ERR_MSGS */

/** Index name prefix in fast index creation, as a string constant */
#define TEMP_INDEX_PREFIX_STR "\377"


#ifndef UNIV_HOTBACKUP
#if defined(HAVE_PAUSE_INSTRUCTION)
/* According to the gcc info page, asm volatile means that the
instruction has important side-effects and must not be removed.
Also asm volatile may trigger a memory barrier (spilling all registers
to memory). */
#ifdef __SUNPRO_CC
#define UT_RELAX_CPU() asm("pause")
#else
#define UT_RELAX_CPU() __asm__ __volatile__("pause")
#endif /* __SUNPRO_CC */

#elif defined(HAVE_FAKE_PAUSE_INSTRUCTION)
#define UT_RELAX_CPU() __asm__ __volatile__("rep; nop")
#elif defined _WIN32
/* In the Win32 API, the x86 PAUSE instruction is executed by calling
the YieldProcessor macro defined in WinNT.h. It is a CPU architecture-
independent way by using YieldProcessor. */
#define UT_RELAX_CPU() YieldProcessor()
#else
#define UT_RELAX_CPU() __asm__ __volatile__("" ::: "memory")
#endif

#if defined(HAVE_HMT_PRIORITY_INSTRUCTION)
#define UT_LOW_PRIORITY_CPU() __asm__ __volatile__("or 1,1,1")
#define UT_RESUME_PRIORITY_CPU() __asm__ __volatile__("or 2,2,2")
#else
#define UT_LOW_PRIORITY_CPU() ((void)0)
#define UT_RESUME_PRIORITY_CPU() ((void)0)
#endif

/** Delays execution for at most max_wait_us microseconds or returns earlier
 if cond becomes true.
 @param cond in: condition to wait for; evaluated every 2 ms
 @param max_wait_us in: maximum delay to wait, in microseconds */
#define UT_WAIT_FOR(cond, max_wait_us)                               \
  do {                                                               \
    uintmax_t start_us;                                              \
    start_us = ut_time_us(NULL);                                     \
    while (!(cond) && ut_time_us(NULL) - start_us < (max_wait_us)) { \
      os_thread_sleep(2000 /* 2 ms */);                              \
    }                                                                \
  } while (0)
#else                  /* !UNIV_HOTBACKUP */
#define UT_RELAX_CPU() /* No op */
#endif                 /* !UNIV_HOTBACKUP */

#define ut_max std::max
#define ut_min std::min


/** Determine how many bytes (groups of 8 bits) are needed to
store the given number of bits.
@param b in: bits
@return number of bytes (octets) needed to represent b */
#define UT_BITS_IN_BYTES(b) (((b) + 7) / 8)


namespace ut {
/** The current value of @@innodb_spin_wait_pause_multiplier. Determines
how many PAUSE instructions to emit for each requested unit of delay
when calling `ut_delay(delay)`. The default value of 50 causes `delay*50` PAUSES
which was equivalent to `delay` microseconds on 100 MHz Pentium + Visual C++.
Useful on processors which have "non-standard" duration of a single PAUSE
instruction - one can compensate for longer PAUSES by setting the
spin_wait_pause_multiplier to a smaller value on such machine */
extern ulong spin_wait_pause_multiplier;
}  // namespace ut

/** Runs an idle loop on CPU. The argument gives the desired delay
 in microseconds on 100 MHz Pentium + Visual C++.
 The actual duration depends on a product of `delay` and the current value of
 @@innodb_spin_wait_pause_multiplier.
 @param[in]   delay   delay in microseconds on 100 MHz Pentium, assuming
                      spin_wait_pause_multiplier is 50 (default).
 @return dummy value */
ulint ut_delay(ulint delay);

/* Forward declaration of transaction handle */
struct trx_t;

/** Get a fixed-length string, quoted as an SQL identifier.
If the string contains a slash '/', the string will be
output as two identifiers separated by a period (.),
as in SQL database_name.identifier.
 @param		[in]	trx		transaction (NULL=no quotes).
 @param		[in]	name		table name.
 @retval	String quoted as an SQL identifier.
*/
std::string ut_get_name(const trx_t *trx, const char *name);

/** Outputs a fixed-length string, quoted as an SQL identifier.
 If the string contains a slash '/', the string will be
 output as two identifiers separated by a period (.),
 as in SQL database_name.identifier. */
void ut_print_name(FILE *f,           /*!< in: output stream */
                   const trx_t *trx,  /*!< in: transaction */
                   const char *name); /*!< in: table name to print */

/** Format a table name, quoted as an SQL identifier.
If the name contains a slash '/', the result will contain two
identifiers separated by a period (.), as in SQL
database_name.table_name.
@see table_name_t
@param[in]	name		table or index name
@param[out]	formatted	formatted result, will be NUL-terminated
@param[in]	formatted_size	size of the buffer in bytes
@return pointer to 'formatted' */
char *ut_format_name(const char *name, char *formatted, ulint formatted_size);

/** Catenate files. */
void ut_copy_file(FILE *dest, /*!< in: output file */
                  FILE *src); /*!< in: input file to be appended to output */

#ifdef _WIN32
/** A substitute for vsnprintf(3), formatted output conversion into
 a limited buffer. Note: this function DOES NOT return the number of
 characters that would have been printed if the buffer was unlimited because
 VC's _vsnprintf() returns -1 in this case and we would need to call
 _vscprintf() in addition to estimate that but we would need another copy
 of "ap" for that and VC does not provide va_copy(). */
void ut_vsnprintf(char *str,       /*!< out: string */
                  size_t size,     /*!< in: str size */
                  const char *fmt, /*!< in: format */
                  va_list ap);     /*!< in: format values */
#else
/** A wrapper for vsnprintf(3), formatted output conversion into
 a limited buffer. Note: this function DOES NOT return the number of
 characters that would have been printed if the buffer was unlimited because
 VC's _vsnprintf() returns -1 in this case and we would need to call
 _vscprintf() in addition to estimate that but we would need another copy
 of "ap" for that and VC does not provide va_copy(). */
#define ut_vsnprintf(buf, size, fmt, ap) ((void)vsnprintf(buf, size, fmt, ap))
#endif /* _WIN32 */


#ifdef UNIV_HOTBACKUP
/** Sprintfs a timestamp to a buffer with no spaces and with ':' characters
replaced by '_'.
@param[in]	buf	buffer where to sprintf */
void meb_sprintf_timestamp_without_extra_chars(char *buf);
#endif /* UNIV_HOTBACKUP */

struct Wait_stats {
  uint64_t wait_loops;

  explicit Wait_stats(uint64_t wait_loops = 0) : wait_loops(wait_loops) {}

  Wait_stats &operator+=(const Wait_stats &rhs) {
    wait_loops += rhs.wait_loops;
    return (*this);
  }

  Wait_stats operator+(const Wait_stats &rhs) const {
    return (Wait_stats{wait_loops + rhs.wait_loops});
  }

  bool any_waits() const { return (wait_loops != 0); }
};

#include "ut0ut.ic"

#endif /* !ut0ut_h */
