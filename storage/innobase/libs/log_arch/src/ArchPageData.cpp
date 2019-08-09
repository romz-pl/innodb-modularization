#include <innodb/log_arch/ArchPageData.h>

#include <innodb/align/ut_align.h>
#include <innodb/allocator/UT_DELETE.h>
#include <innodb/allocator/UT_NEW.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_zalloc.h>
#include <innodb/log_arch/Arch_Block.h>
#include <innodb/log_arch/flags.h>

/** Allocate buffer and initialize blocks
@return true, if successful */
bool ArchPageData::init() {
  uint alloc_size;
  uint index;
  byte *mem_ptr;

  ut_ad(m_buffer == nullptr);

  m_block_size = ARCH_PAGE_BLK_SIZE;
  m_num_data_blocks = ARCH_PAGE_NUM_BLKS;

  /* block size and number must be in power of 2 */
  ut_ad(ut_is_2pow(m_block_size));
  ut_ad(ut_is_2pow(m_num_data_blocks));

  alloc_size = m_block_size * m_num_data_blocks;
  alloc_size += m_block_size;

  /* For reset block. */
  alloc_size += m_block_size;

  /* For partial flush block. */
  alloc_size += m_block_size;

  /* Allocate buffer for memory blocks. */
  m_buffer = static_cast<byte *>(ut_zalloc(alloc_size, mem_key_archive));

  if (m_buffer == nullptr) {
    return (false);
  }

  mem_ptr = static_cast<byte *>(ut_align(m_buffer, m_block_size));

  Arch_Block *cur_blk;

  /* Create memory blocks. */
  for (index = 0; index < m_num_data_blocks; index++) {
    cur_blk = UT_NEW(Arch_Block(mem_ptr, m_block_size, ARCH_DATA_BLOCK),
                     mem_key_archive);

    if (cur_blk == nullptr) {
      return (false);
    }

    m_data_blocks.push_back(cur_blk);
    mem_ptr += m_block_size;
  }

  m_reset_block = UT_NEW(Arch_Block(mem_ptr, m_block_size, ARCH_RESET_BLOCK),
                         mem_key_archive);

  if (m_reset_block == nullptr) {
    return (false);
  }

  mem_ptr += m_block_size;

  m_partial_flush_block = UT_NEW(
      Arch_Block(mem_ptr, m_block_size, ARCH_DATA_BLOCK), mem_key_archive);

  if (m_partial_flush_block == nullptr) {
    return (false);
  }

  return (true);
}

/** Delete blocks and buffer */
void ArchPageData::clean() {
  for (auto &block : m_data_blocks) {
    UT_DELETE(block);
    block = nullptr;
  }

  if (m_reset_block != nullptr) {
    UT_DELETE(m_reset_block);
    m_reset_block = nullptr;
  }

  if (m_partial_flush_block != nullptr) {
    UT_DELETE(m_partial_flush_block);
    m_partial_flush_block = nullptr;
  }

  if (m_buffer != nullptr) {
    ut_free(m_buffer);
  }
}

/** Get the block for a position
@param[in]	pos	position in page archive sys
@param[in]	type	block type
@return page archive in memory block */
Arch_Block *ArchPageData::get_block(Arch_Page_Pos *pos, Arch_Blk_Type type) {
  switch (type) {
    case ARCH_DATA_BLOCK: {
      /* index = block_num % m_num_blocks */
      ut_ad(ut_is_2pow(m_num_data_blocks));

      uint index = pos->m_block_num & (m_num_data_blocks - 1);
      return (m_data_blocks[index]);
    }

    case ARCH_RESET_BLOCK:
      return (m_reset_block);

    default:
      ut_ad(false);
  }

  return (nullptr);
}
