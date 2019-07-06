#include <innodb/memory/mem_block_set_start.h>

#include <innodb/memory/mem_block_info_t.h>

void mem_block_set_start(mem_block_t *block, ulint start) {
  ut_ad(start > 0);

  block->start = start;
}
