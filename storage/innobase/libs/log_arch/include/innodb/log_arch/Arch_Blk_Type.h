#pragma once

#include <innodb/univ/univ.h>

/** Archiver block type */
enum Arch_Blk_Type {
  /* Block which holds reset information */
  ARCH_RESET_BLOCK = 0,

  /* Block which holds archived page IDs */
  ARCH_DATA_BLOCK
};
