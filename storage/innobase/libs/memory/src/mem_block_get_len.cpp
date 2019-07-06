#include <innodb/memory/mem_block_get_len.h>

#include <innodb/memory/mem_block_info_t.h>

ulint mem_block_get_len(mem_block_t *block) {
    return (block->len);
}
