#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_buf_t.h>

#include <cstring>

/** mtr_buf_t copier */
struct mtr_buf_copy_t {
  /** The copied buffer */
  mtr_buf_t m_buf;

  /** Append a block to the redo log buffer.
  @return whether the appending should continue (always true here) */
  bool operator()(const mtr_buf_t::block_t *block) {
    byte *buf = m_buf.open(block->used());
    memcpy(buf, block->begin(), block->used());
    m_buf.close(buf + block->used());
    return (true);
  }
};
