#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Blk_Type.h>
#include <innodb/log_arch/Arch_Blk_Flush_Type.h>

/** Doublewrite buffer block along with their info. */
struct Arch_Dblwr_Block {
  /** Type of block flushed into the doublewrite block */
  Arch_Blk_Type m_block_type;

  /** Flush type of the block flushed into the doublewrite buffer */
  Arch_Blk_Flush_Type m_flush_type;

  /** Block number of the block flushed into the doublewrite buffer */
  uint64_t m_block_num;

  /** Doublewrite buffer block */
  byte *m_block;
};
