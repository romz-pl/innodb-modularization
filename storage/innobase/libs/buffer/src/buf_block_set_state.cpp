#include <innodb/buffer/buf_block_set_state.h>

#include <innodb/buf_page/buf_page_set_state.h>
#include <innodb/buf_page/buf_page_state.h>
#include <innodb/buffer/buf_block_t.h>

/** Sets the state of a block. */
void buf_block_set_state(
    buf_block_t *block,        /*!< in/out: pointer to control block */
    buf_page_state state) /*!< in: state */
{
  buf_page_set_state(&block->page, state);
}
