#pragma once

#include <innodb/univ/univ.h>

/** Clone handle type */
enum Clone_Handle_Type {
  /** Clone Handle for COPY */
  CLONE_HDL_COPY = 1,

  /** Clone Handle for APPLY */
  CLONE_HDL_APPLY
};
