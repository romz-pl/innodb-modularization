#pragma once

#include <innodb/univ/univ.h>

#include <innodb/counter/counter.h>
#include <innodb/atomic/atomic.h>

/** Blocks for doing IO, used in the transparent compression
and encryption code. */
struct Block {
  /** Default constructor */
  Block() : m_ptr(), m_in_use() {}

  byte *m_ptr;

  byte pad[INNOBASE_CACHE_LINE_SIZE - sizeof(ulint)];
  lock_word_t m_in_use;
};

