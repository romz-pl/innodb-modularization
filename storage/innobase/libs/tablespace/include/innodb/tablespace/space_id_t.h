#pragma once

#include <innodb/univ/univ.h>

#include <limits>

/** Tablespace identifier */
typedef uint32_t space_id_t;

/** Unknown space id */
constexpr space_id_t SPACE_UNKNOWN = std::numeric_limits<space_id_t>::max();
