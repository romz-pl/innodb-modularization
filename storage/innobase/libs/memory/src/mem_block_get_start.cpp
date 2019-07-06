#include <innodb/memory/mem_block_get_start.h>

#include <innodb/memory/mem_block_info_t.h>

ulint mem_block_get_start(mem_block_t *block) {
    return (block->start);
}
