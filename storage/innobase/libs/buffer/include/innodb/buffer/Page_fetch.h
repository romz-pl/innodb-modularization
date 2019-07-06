#pragma once

#include <innodb/univ/univ.h>

/** @name Modes for buf_page_get_gen */
/* @{ */
enum class Page_fetch {
  /** Get always */
  NORMAL,

  /** Same as NORMAL, but hint that the fetch is part of a large scan.
  Try not to flood the buffer pool with pages that may not be accessed again
  any time soon. */
  SCAN,

  /** get if in pool */
  IF_IN_POOL,

  /** get if in pool, do not make the block young in the LRU list */
  PEEK_IF_IN_POOL,

  /** get and bufferfix, but set no latch; we have separated this case, because
  it is error-prone programming not to set a latch, and it  should be used with
  care */
  NO_LATCH,

  /** Get the page only if it's in the buffer pool, if not then set a watch on
  the page. */
  IF_IN_POOL_OR_WATCH,

  /** Like Page_fetch::NORMAL, but do not mind if the file page has been
  freed. */
  POSSIBLY_FREED
};
/* @} */
