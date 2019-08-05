#include <innodb/memory/mem_heap_create_func.h>

#include <innodb/memory/macros.h>
#include <innodb/memory/mem_heap_create_block.h>
#include <innodb/lst/lst.h>

/** Creates a memory heap.
NOTE: Use the corresponding macros instead of this function.
A single user buffer of 'size' will fit in the block.
0 creates a default size block.
@param[in]	size		Desired start block size.
@param[in]	file_name	File name where created
@param[in]	line		Line where created
@param[in]	type		Heap type
@return own: memory heap, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
mem_heap_t *mem_heap_create_func(ulint size,
#ifdef UNIV_DEBUG
                                 const char *file_name, ulint line,
#endif /* UNIV_DEBUG */
                                 ulint type) {
  mem_block_t *block;

  if (!size) {
    size = MEM_BLOCK_START_SIZE;
  }

  block = mem_heap_create_block(NULL, size, type, file_name, line);

  if (block == NULL) {
    return (NULL);
  }

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY
  /* The first block should not be in buffer pool,
  because it might be relocated to resize buffer pool. */
  ut_ad(block->buf_block == NULL);
#endif /* !UNIV_LIBRARY */
#endif /* UNIV_HOTBACKUP */

  UT_LIST_INIT(block->base, &mem_block_t::list);

  /* Add the created block itself as the first block in the list */
  UT_LIST_ADD_FIRST(block->base, block);

  return (block);
}
