#pragma once

#include <innodb/univ/univ.h>


/** Archiver block flush type */
enum Arch_Blk_Flush_Type {
  /** Flush when block is full */
  ARCH_FLUSH_NORMAL = 0,

  /** Flush partial block.
  Needed for persistent page tracking. */
  ARCH_FLUSH_PARTIAL
};
