#pragma once

#include <innodb/univ/univ.h>

/** Statistics of buddy blocks of a given size. */
struct buf_buddy_stat_t {
  /** Number of blocks allocated from the buddy system. */
  ulint used;
  /** Number of blocks relocated by the buddy system. */
  uint64_t relocated;
  /** Total duration of block relocations, in microseconds. */
  uint64_t relocated_usec;
};
