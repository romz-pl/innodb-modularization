#include <innodb/buffer/buf_block_get_io_fix_unlocked.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_get_io_fix_unlocked.h>
#include <innodb/buf_block/buf_block_t.h>

/** Gets the io_fix state of a buffer block. Does not assert that the
buf_page_get_mutex() mutex is held, to be used in the cases where it is safe
not to hold it.
@param[in]	block	pointer to the buffer block
@return page io_fix state */
buf_io_fix buf_block_get_io_fix_unlocked(const buf_block_t *block) {
  return (buf_page_get_io_fix_unlocked(&block->page));
}

#endif
