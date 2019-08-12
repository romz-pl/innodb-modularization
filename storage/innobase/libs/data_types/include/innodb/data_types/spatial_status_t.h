#pragma once

#include <innodb/univ/univ.h>

/** whether a col is used in spatial index or regular index
Note: the spatial status is part of persistent undo log,
so we should not modify the values in MySQL 5.7 */
enum spatial_status_t {
  /* Unkown status (undo format in 5.7.9) */
  SPATIAL_UNKNOWN = 0,

  /** Not used in gis index. */
  SPATIAL_NONE = 1,

  /** Used in both spatial index and regular index. */
  SPATIAL_MIXED = 2,

  /** Only used in spatial index. */
  SPATIAL_ONLY = 3
};
