#include <innodb/buf_page/buf_page_in_file.h>

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buf_page/buf_page_get_state.h>
#include <innodb/error/ut_error.h>

/** Determines if a block is mapped to a tablespace.
 @return true if mapped */
ibool buf_page_in_file(
    const buf_page_t *bpage) /*!< in: pointer to control block */
{
  switch (buf_page_get_state(bpage)) {
    case BUF_BLOCK_POOL_WATCH:
      ut_error;
      break;
    case BUF_BLOCK_ZIP_PAGE:
    case BUF_BLOCK_ZIP_DIRTY:
    case BUF_BLOCK_FILE_PAGE:
      return (TRUE);
    case BUF_BLOCK_NOT_USED:
    case BUF_BLOCK_READY_FOR_USE:
    case BUF_BLOCK_MEMORY:
    case BUF_BLOCK_REMOVE_HASH:
      break;
  }

  return (FALSE);
}
