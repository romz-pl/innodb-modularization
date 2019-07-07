#include <innodb/buffer/buf_block_get_state.h>

#include <innodb/buffer/buf_block_t.h>
#include <innodb/buffer/buf_page_state.h>
#include <innodb/buffer/buf_page_get_state.h>

/** Gets the state of a block.
 @return state */
buf_page_state buf_block_get_state(
    const buf_block_t *block) /*!< in: pointer to the control block */
{
  return (buf_page_get_state(&block->page));
}
