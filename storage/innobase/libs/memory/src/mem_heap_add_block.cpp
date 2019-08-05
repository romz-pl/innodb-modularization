#include <innodb/memory/mem_heap_add_block.h>

#include <innodb/memory/mem_block_validate.h>
#include <innodb/memory/mem_block_get_len.h>
#include <innodb/memory/mem_heap_create_block.h>
#include <innodb/memory/macros.h>

/** Adds a new block to a memory heap.
 @return created block, NULL if did not succeed (only possible for
 MEM_HEAP_BTR_SEARCH type heaps) */
mem_block_t *mem_heap_add_block(mem_heap_t *heap, /*!< in: memory heap */
                                ulint n) /*!< in: number of bytes user needs */
{
  mem_block_t *block;
  mem_block_t *new_block;
  ulint new_size;

  ut_d(mem_block_validate(heap));

  block = UT_LIST_GET_LAST(heap->base);

  /* We have to allocate a new block. The size is always at least
  doubled until the standard size is reached. After that the size
  stays the same, except in cases where the caller needs more space. */

  new_size = 2 * mem_block_get_len(block);

  if (heap->type != MEM_HEAP_DYNAMIC) {
    /* From the buffer pool we allocate buffer frames */
    ut_a(n <= MEM_MAX_ALLOC_IN_BUF);

    if (new_size > MEM_MAX_ALLOC_IN_BUF) {
      new_size = MEM_MAX_ALLOC_IN_BUF;
    }
  } else if (new_size > MEM_BLOCK_STANDARD_SIZE) {
    new_size = MEM_BLOCK_STANDARD_SIZE;
  }

  if (new_size < n) {
    new_size = n;
  }

  new_block = mem_heap_create_block(heap, new_size, heap->type, heap->file_name,
                                    heap->line);
  if (new_block == NULL) {
    return (NULL);
  }

  /* Add the new block as the last block */

  UT_LIST_INSERT_AFTER(heap->base, block, new_block);

  return (new_block);
}
