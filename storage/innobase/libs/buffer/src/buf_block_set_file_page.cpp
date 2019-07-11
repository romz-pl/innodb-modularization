#include <innodb/buffer/buf_block_set_file_page.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_block_t.h>
#include <innodb/tablespace/page_id_t.h>
#include <innodb/buffer/buf_block_set_state.h>

/** Map a block to a file page.
@param[in,out]	block	pointer to control block
@param[in]	page_id	page id */
void buf_block_set_file_page(buf_block_t *block, const page_id_t &page_id) {
  buf_block_set_state(block, BUF_BLOCK_FILE_PAGE);
  block->page.id.copy_from(page_id);
}

#endif
