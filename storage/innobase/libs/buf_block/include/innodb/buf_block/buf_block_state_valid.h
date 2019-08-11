#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_block/buf_block_get_state.h>
#include <innodb/buf_page/buf_page_state.h>

/** Check if a buf_block_t object is in a valid state
@param block buffer block
@return true if valid */
#define buf_block_state_valid(block)                   \
  (buf_block_get_state(block) >= BUF_BLOCK_NOT_USED && \
   (buf_block_get_state(block) <= BUF_BLOCK_REMOVE_HASH))
