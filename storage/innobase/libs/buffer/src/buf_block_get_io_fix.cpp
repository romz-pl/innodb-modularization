#include <innodb/buffer/buf_block_get_io_fix.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_get_io_fix.h>
#include <innodb/buf_block/buf_block_t.h>

/** Gets the io_fix state of a block.
 @return io_fix state */
buf_io_fix buf_block_get_io_fix(
    const buf_block_t *block) /*!< in: pointer to the control block */
{
  return (buf_page_get_io_fix(&block->page));
}


#endif
