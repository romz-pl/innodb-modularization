#include <innodb/buffer/buf_block_get_freed_page_clock.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_get_freed_page_clock.h>
#include <innodb/buffer/buf_block_t.h>

/** Reads the freed_page_clock of a buffer block.
 @return freed_page_clock */
ulint buf_block_get_freed_page_clock(const buf_block_t *block) /*!< in: block */
{
  return (buf_page_get_freed_page_clock(&block->page));
}

#endif
