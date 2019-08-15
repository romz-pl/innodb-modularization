#pragma once

#include <innodb/univ/univ.h>

/** Persistent dynamic metadata type, there should be 1 to 1
relationship between the metadata and the type. Please keep them in order
so that we can iterate over it */
enum persistent_type_t {
  /** The smallest type, which should be 1 less than the first
  true type */
  PM_SMALLEST_TYPE = 0,

  /** Persistent Metadata type for corrupted indexes */
  PM_INDEX_CORRUPTED = 1,

  /** Persistent Metadata type for autoinc counter */
  PM_TABLE_AUTO_INC = 2,

  /* TODO: Will add following types
  PM_TABLE_UPDATE_TIME = 3,
  Maybe something tablespace related
  PM_TABLESPACE_SIZE = 4,
  PM_TABLESPACE_MAX_TRX_ID = 5, */

  /** The biggest type, which should be 1 bigger than the last
  true type */
  PM_BIGGEST_TYPE = 3
};
