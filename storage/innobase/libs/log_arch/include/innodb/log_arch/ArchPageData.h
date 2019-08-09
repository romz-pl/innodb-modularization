#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Block_Vec.h>
#include <innodb/log_arch/Arch_Blk_Type.h>

struct Arch_Page_Pos;

/** Page archiver in memory data */
struct ArchPageData {
  /** Constructor */
  ArchPageData() {}

  /** Allocate buffer and initialize blocks
  @return true, if successful */
  bool init();

  /** Delete blocks and buffer */
  void clean();

  /** Get the block for a position
  @param[in]	pos	position in page archive sys
  @param[in]	type	block type
  @return page archive in memory block */
  Arch_Block *get_block(Arch_Page_Pos *pos, Arch_Blk_Type type);

  /** @return temporary block used to copy active block for partial flush. */
  Arch_Block *get_partial_flush_block() const {
    return (m_partial_flush_block);
  }

  /** Vector of data blocks */
  Arch_Block_Vec m_data_blocks{};

  /** Reset block */
  Arch_Block *m_reset_block{nullptr};

  /** Temporary block used to copy active block for partial flush. */
  Arch_Block *m_partial_flush_block{nullptr};

  /** Block size in bytes */
  uint m_block_size{};

  /** Total number of blocks */
  uint m_num_data_blocks{};

  /** In memory buffer */
  byte *m_buffer{nullptr};
};
