#pragma once

#include <innodb/univ/univ.h>

/** Type used for all log sequence number storage and arithmetics. */
typedef uint64_t lsn_t;

/** Maximum possible lsn value is slightly higher than the maximum sn value,
because lsn sequence enumerates also bytes used for headers and footers of
all log blocks. However, still 64-bits are enough to represent the maximum
lsn value, because only 63 bits are used to represent sn value. */
constexpr lsn_t LSN_MAX = (1ULL << 63) - 1;

