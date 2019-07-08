#pragma once

#include <innodb/univ/univ.h>

#ifndef __STDC_LIMIT_MACROS
/* Required for FreeBSD so that INT64_MAX is defined. */
#define __STDC_LIMIT_MACROS
#endif /* __STDC_LIMIT_MACROS */

#include <stdint.h>

/** Counter minimum value is initialized to be max value of mon_type_t (int64_t) */
#define MIN_RESERVED INT64_MAX
#define MAX_RESERVED (~MIN_RESERVED)


#ifndef UNIV_HOTBACKUP

/** Number of bit in a ulint datatype */
#define NUM_BITS_ULINT (sizeof(ulint) * CHAR_BIT)

#define MONITOR_INIT_ZERO_VALUE 0

#endif
