#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/gis_type/rtr_info_active.h>

/* Tracking structure for all onoging search for an index */
typedef struct rtr_info_track {
  rtr_info_active *rtr_active; /*!< Active search info */
  ib_mutex_t rtr_active_mutex;
  /*!< mutex to protect
  rtr_active */
} rtr_info_track_t;
