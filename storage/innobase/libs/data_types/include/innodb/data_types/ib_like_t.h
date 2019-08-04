#pragma once

#include <innodb/univ/univ.h>

/** SQL Like operator comparison types */
enum ib_like_t {
  IB_LIKE_EXACT, /**< e.g.  STRING */
  IB_LIKE_PREFIX /**< e.g., STRING% */
};
