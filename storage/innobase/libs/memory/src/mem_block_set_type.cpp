#include <innodb/memory/mem_block_set_type.h>

#include <innodb/memory/mem_block_info_t.h>

void mem_block_set_type(mem_block_t *block, ulint type) {
  ut_ad((type == MEM_HEAP_DYNAMIC) || (type == MEM_HEAP_BUFFER) ||
        (type == MEM_HEAP_BUFFER + MEM_HEAP_BTR_SEARCH));

  block->type = type;
}
