#include <innodb/memory/mem_block_set_len.h>

#include <innodb/memory/mem_block_info_t.h>

void mem_block_set_len(mem_block_t *block, ulint len) {
  ut_ad(len > 0);

  block->len = len;
}
