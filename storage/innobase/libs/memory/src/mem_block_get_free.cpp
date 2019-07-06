#include <innodb/memory/mem_block_get_free.h>

#include <innodb/memory/mem_block_info_t.h>

ulint mem_block_get_free(mem_block_t *block) {
    return (block->free);
}
