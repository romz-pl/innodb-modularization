#include <innodb/buffer/buf_block_set_io_fix.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_set_io_fix.h>
#include <innodb/buf_block/buf_block_t.h>

/** Sets the io_fix state of a block. */
void buf_block_set_io_fix(buf_block_t *block,     /*!< in/out: control block */
                          buf_io_fix io_fix) /*!< in: io_fix state */
{
  buf_page_set_io_fix(&block->page, io_fix);
}
#endif
