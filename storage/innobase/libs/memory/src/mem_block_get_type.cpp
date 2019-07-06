#include <innodb/memory/mem_block_get_type.h>

#include <innodb/memory/mem_block_info_t.h>

ulint mem_block_get_type(mem_block_t *block) {
    return (block->type);
}
