#include <innodb/log_arch/Arch_Dblwr_Ctx.h>

#include <innodb/allocator/UT_NEW_ARRAY_NOKEY.h>
#include <innodb/log_arch/flags.h>
#include <innodb/log_arch/Arch_Block.h>
#include <innodb/log_arch/Arch_Page_Dblwr_Offset.h>

dberr_t Arch_Dblwr_Ctx::init(const char *dblwr_path,
                             const char *dblwr_base_file, uint dblwr_num_files,
                             uint64_t dblwr_file_size) {
  m_file_size = dblwr_file_size;

  m_buf = static_cast<byte *>(UT_NEW_ARRAY_NOKEY(byte, m_file_size));

  if (m_buf == nullptr) {
    return (DB_OUT_OF_MEMORY);
  }

  memset(m_buf, 0, m_file_size);

  auto err = m_file_ctx.init(dblwr_path, nullptr, dblwr_base_file,
                             dblwr_num_files, m_file_size);

  return (err);
}

dberr_t Arch_Dblwr_Ctx::read_blocks() {
  ut_ad(m_buf != nullptr);

  auto err = m_file_ctx.open(true, LSN_MAX, 0, 0);

  if (err != DB_SUCCESS) {
    return (err);
  }

  ut_ad(m_file_ctx.get_phy_size() == m_file_size);

  /* Read the entire file. */
  err = m_file_ctx.read(m_buf, 0, m_file_size);

  if (err != DB_SUCCESS) {
    return (err);
  }

  Arch_Dblwr_Block dblwr_block;

  for (uint block_num = 0; block_num < m_file_size / ARCH_PAGE_BLK_SIZE;
       ++block_num) {
    auto block = m_buf + (block_num * ARCH_PAGE_BLK_SIZE);

    if (!Arch_Block::validate(block)) {
      continue;
    }

    if (block_num == ARCH_PAGE_DBLWR_RESET_PAGE) {
      dblwr_block.m_block_type = ARCH_RESET_BLOCK;
      dblwr_block.m_flush_type = ARCH_FLUSH_NORMAL;
    } else {
      dblwr_block.m_block_type = ARCH_DATA_BLOCK;
      dblwr_block.m_flush_type = (block_num == ARCH_PAGE_DBLWR_FULL_FLUSH_PAGE)
                                     ? ARCH_FLUSH_NORMAL
                                     : ARCH_FLUSH_PARTIAL;
    }

    dblwr_block.m_block_num = Arch_Block::get_block_number(block);
    dblwr_block.m_block = block;
    m_blocks.push_back(dblwr_block);
  }

  m_file_ctx.close();

  return (err);
}
