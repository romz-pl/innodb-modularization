#include <innodb/log_arch/Arch_Page_Pos.h>

#include <innodb/log_arch/flags.h>


/** Initialize a position */
void Arch_Page_Pos::init() {
  m_block_num = 0;
  m_offset = ARCH_PAGE_BLK_HEADER_LENGTH;
}

/** Position in the beginning of next block */
void Arch_Page_Pos::set_next() {
  m_block_num++;
  m_offset = ARCH_PAGE_BLK_HEADER_LENGTH;
}
