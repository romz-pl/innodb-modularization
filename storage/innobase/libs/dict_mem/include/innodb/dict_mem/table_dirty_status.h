#pragma once

#include <innodb/univ/univ.h>

/** The dirty status of tables, used to indicate if a table has some
dynamic metadata changed to be written back */
enum table_dirty_status {
  /** Some persistent metadata is now dirty in memory, need to be
  written back to DDTableBuffer table and(or directly to) DD table.
  There could be some exceptions, when it's marked as dirty, but
  the metadata has already been written back to DDTableBuffer.
  For example, if a corrupted index is found and marked as corrupted,
  then it gets dropped. At this time, the dirty_status is still of
  this dirty value. Also a concurrent checkpoint make this bit
  out-of-date for other working threads, which still think the
  status is dirty and write-back is necessary.
  There could be either one row or no row for this table in
  DDTableBuffer table */
  METADATA_DIRTY = 0,
  /** Some persistent metadata is buffered in DDTableBuffer table,
  need to be written back to DD table. There is must be one row in
  DDTableBuffer table for this table */
  METADATA_BUFFERED,
  /** All persistent metadata are up to date. There is no row
  for this table in DDTableBuffer table */
  METADATA_CLEAN
};
