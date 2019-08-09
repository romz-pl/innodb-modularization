#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Page_Pos.h>
#include <innodb/log_types/lsn_t.h>

/** Structure which represents a point in a file. */
struct Arch_Point {
  /** LSN of the point */
  lsn_t lsn{LSN_MAX};

  /** Position of the point */
  Arch_Page_Pos pos;
};
