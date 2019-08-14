#pragma once

#include <innodb/univ/univ.h>

/// In memory representation of a minimum bounding rectangle
typedef struct rtr_mbr {
  /// minimum on x
  double xmin;
  /// maximum on x
  double xmax;
  /// minimum on y
  double ymin;
  /// maximum on y
  double ymax;
} rtr_mbr_t;
