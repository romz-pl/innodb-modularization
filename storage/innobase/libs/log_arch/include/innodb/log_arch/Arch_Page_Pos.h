#pragma once

#include <innodb/univ/univ.h>

/** Position in page ID archiving system */
struct Arch_Page_Pos {
  /** Initialize a position */
  void init();

  /** Position in the beginning of next block */
  void set_next();

  /** Unique block number */
  uint64_t m_block_num;

  /** Offset within a block */
  uint m_offset;

  bool operator<(Arch_Page_Pos pos) {
    if (m_block_num < pos.m_block_num ||
        (m_block_num == pos.m_block_num && m_offset <= pos.m_offset)) {
      return (true);
    }
    return (false);
  }
};
