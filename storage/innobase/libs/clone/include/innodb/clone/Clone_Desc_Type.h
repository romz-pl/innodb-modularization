#pragma once

#include <innodb/univ/univ.h>

/** Clone descriptors contain meta information needed for applying cloned data.
These are PODs with interface to serialize and deserialize them. */
enum Clone_Desc_Type {
  /** Logical pointer to identify a clone operation */
  CLONE_DESC_LOCATOR = 1,

  /** Metadata for a Task/Thread for clone operation */
  CLONE_DESC_TASK_METADATA,

  /** Information for snapshot state */
  CLONE_DESC_STATE,

  /** Metadata for a database file */
  CLONE_DESC_FILE_METADATA,

  /** Information for a data block */
  CLONE_DESC_DATA,

  /** Must be the last member */
  CLONE_DESC_MAX
};
