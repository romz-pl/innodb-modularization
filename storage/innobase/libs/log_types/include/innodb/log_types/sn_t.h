#pragma once

#include <innodb/univ/univ.h>

/** Type used for sn values, which enumerate bytes of data stored in the log.
Note that these values skip bytes of headers and footers of log blocks. */
typedef uint64_t sn_t;
