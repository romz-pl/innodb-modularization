#pragma once

#include <innodb/univ/univ.h>

/** Quiescing states for flushing tables to disk. */
enum ib_quiesce_t {
  QUIESCE_NONE,
  QUIESCE_START,   /*!< Initialise, prepare to start */
  QUIESCE_COMPLETE /*!< All done */
};
