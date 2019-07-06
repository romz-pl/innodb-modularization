#pragma once

#include <innodb/univ/univ.h>

/** File operations for tablespace */
enum fil_operation_t {

  /** delete a single-table tablespace */
  FIL_OPERATION_DELETE,

  /** close a single-table tablespace */
  FIL_OPERATION_CLOSE
};
