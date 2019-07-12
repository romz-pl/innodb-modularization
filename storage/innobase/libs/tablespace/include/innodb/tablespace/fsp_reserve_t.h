#pragma once

#include <innodb/univ/univ.h>

/** Flags for fsp_reserve_free_extents */
enum fsp_reserve_t {
  FSP_NORMAL,   /* reservation during normal B-tree operations */
  FSP_UNDO,     /* reservation done for undo logging */
  FSP_CLEANING, /* reservation done during purge operations */
  FSP_BLOB      /* reservation being done for BLOB insertion */
};
