#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lst/lst.h>
#include <innodb/disk/flags.h>
#include <innodb/buf_page/buf_page_t.h>

/** Struct that is embedded in the free zip blocks */
struct buf_buddy_free_t {
  union {
    ulint size; /*!< size of the block */
    byte bytes[FIL_PAGE_DATA];
    /*!< stamp[FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID]
    == BUF_BUDDY_FREE_STAMP denotes a free
    block. If the space_id field of buddy
    block != BUF_BUDDY_FREE_STAMP, the block
    is not in any zip_free list. If the
    space_id is BUF_BUDDY_FREE_STAMP then
    stamp[0] will contain the
    buddy block size. */
  } stamp;

  buf_page_t bpage; /*!< Embedded bpage descriptor */
  UT_LIST_NODE_T(buf_buddy_free_t) list;
  /*!< Node of zip_free list */
};
