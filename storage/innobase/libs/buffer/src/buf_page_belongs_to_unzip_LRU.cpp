#include <innodb/buffer/buf_page_belongs_to_unzip_LRU.h>

#include <innodb/buffer/buf_page_in_file.h>
#include <innodb/buffer/buf_page_get_state.h>
#include <innodb/buf_page/buf_page_t.h>

#ifndef UNIV_HOTBACKUP

/** Determines if a block should be on unzip_LRU list.
 @return true if block belongs to unzip_LRU */
ibool buf_page_belongs_to_unzip_LRU(
    const buf_page_t *bpage) /*!< in: pointer to control block */
{
  ut_ad(buf_page_in_file(bpage));

  return (bpage->zip.data && buf_page_get_state(bpage) == BUF_BLOCK_FILE_PAGE);
}

#endif
