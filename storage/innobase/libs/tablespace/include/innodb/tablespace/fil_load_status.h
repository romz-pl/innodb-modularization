#pragma once

#include <innodb/univ/univ.h>

enum fil_load_status {
  /** The tablespace file(s) were found and valid. */
  FIL_LOAD_OK,

  /** The name no longer matches space_id */
  FIL_LOAD_ID_CHANGED,

  /** The file(s) were not found */
  FIL_LOAD_NOT_FOUND,

  /** The file(s) were not valid */
  FIL_LOAD_INVALID,

  /** The tablespace file ID in the first page doesn't match
  expected value. */
  FIL_LOAD_MISMATCH
};
