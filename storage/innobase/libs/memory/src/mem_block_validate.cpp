#include <innodb/memory/mem_block_validate.h>

#include <innodb/memory/mem_block_info_t.h>
#include <innodb/memory/macros.h>
#include <innodb/logger/fatal.h>

/** Checks that an object is a memory heap block
@param[in]	block	Memory block to check. */
void mem_block_validate(const mem_block_t *block) {
  if (block->magic_n != MEM_BLOCK_MAGIC_N) {
    ib::fatal error;
    error << "Memory block is invalid (correctness field value"
             " should be "
          << MEM_BLOCK_MAGIC_N << ", but it is " << block->magic_n
          << ") in a block of size " << block->len;
    ut_d(error << ", allocated at " << block->file_name << ":" << block->line);
  }
}
