#include <innodb/buffer/buf_page_get_state.h>

#include <innodb/buffer/buf_page_t.h>
#include <innodb/buffer/buf_page_state.h>

/** Gets the state of a block.
 @return state */
buf_page_state buf_page_get_state(
    const buf_page_t *bpage) /*!< in: pointer to the control block */
{
  enum buf_page_state state = bpage->state;

#ifdef UNIV_DEBUG
  switch (state) {
    case BUF_BLOCK_POOL_WATCH:
    case BUF_BLOCK_ZIP_PAGE:
    case BUF_BLOCK_ZIP_DIRTY:
    case BUF_BLOCK_NOT_USED:
    case BUF_BLOCK_READY_FOR_USE:
    case BUF_BLOCK_FILE_PAGE:
    case BUF_BLOCK_MEMORY:
    case BUF_BLOCK_REMOVE_HASH:
      break;
    default:
      ut_error;
  }
#endif /* UNIV_DEBUG */

  return (state);
}
