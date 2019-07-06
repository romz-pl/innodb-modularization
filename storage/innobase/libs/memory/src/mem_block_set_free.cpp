#include <innodb/memory/mem_block_set_free.h>

#include <innodb/memory/mem_block_info_t.h>
#include <innodb/memory/mem_block_get_len.h>

void mem_block_set_free(mem_block_t *block, ulint free) {
  ut_ad(free >= block->start);
  ut_ad(free <= mem_block_get_len(block));

  block->free = free;
}
