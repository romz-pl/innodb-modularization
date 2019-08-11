#pragma once

#include <innodb/univ/univ.h>

/* @{ */
enum class Cache_hint {
  /** Move the block to the start of the LRU list if there is a danger that the
  block would drift out of the buffer  pool*/
  MAKE_YOUNG = 51,

  /** Preserve the current LRU position of the block. */
  KEEP_OLD = 52
};

/* @} */
