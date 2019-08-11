#include <innodb/buf_chunk/buf_chunk_not_freed.h>

#include <innodb/buf_block/buf_block_t.h>
#include <innodb/buf_chunk/buf_chunk_t.h>
#include <innodb/buf_block/buf_block_get_state.h>
#include <innodb/buffer/buf_flush_ready_for_replace.h>
#include <innodb/buf_page/buf_page_mutex_enter.h>
#include <innodb/buf_page/buf_page_mutex_exit.h>
#include <innodb/error/ut_error.h>

/** Checks that all file pages in the buffer chunk are in a replaceable state.
 @return address of a non-free block, or NULL if all freed */
const buf_block_t *buf_chunk_not_freed(
    buf_chunk_t *chunk) /*!< in: chunk being checked */
{
  buf_block_t *block;
  ulint i;

  block = chunk->blocks;

  for (i = chunk->size; i--; block++) {
    ibool ready;

    switch (buf_block_get_state(block)) {
      case BUF_BLOCK_POOL_WATCH:
      case BUF_BLOCK_ZIP_PAGE:
      case BUF_BLOCK_ZIP_DIRTY:
        /* The uncompressed buffer pool should never
        contain compressed block descriptors. */
        ut_error;
        break;
      case BUF_BLOCK_NOT_USED:
      case BUF_BLOCK_READY_FOR_USE:
      case BUF_BLOCK_MEMORY:
      case BUF_BLOCK_REMOVE_HASH:
        /* Skip blocks that are not being used for
        file pages. */
        break;
      case BUF_BLOCK_FILE_PAGE:
        buf_page_mutex_enter(block);
        ready = buf_flush_ready_for_replace(&block->page);
        buf_page_mutex_exit(block);

        if (!ready) {
          return (block);
        }

        break;
    }
  }

  return (NULL);
}
