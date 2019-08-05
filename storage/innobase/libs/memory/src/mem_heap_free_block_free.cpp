#include <innodb/memory/mem_heap_free_block_free.h>

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY

struct buf_block_t;
void buf_block_free(buf_block_t *block);


/** Frees the free_block field from a memory heap. */
void mem_heap_free_block_free(mem_heap_t *heap) /*!< in: heap */
{
  if (UNIV_LIKELY_NULL(heap->free_block)) {
#ifdef UNIV_DEBUG_VALGRIND
    void *block = static_cast<buf_block_t *>(heap->free_block)->frame;
    /* Make memory available again for the buffer pool, since
    we previously set parts of the block to "free" state in
    heap allocator. */
    UNIV_MEM_ALLOC(block, UNIV_PAGE_SIZE);
#endif

    buf_block_free(static_cast<buf_block_t *>(heap->free_block));

    heap->free_block = NULL;
  }
}
#endif /* !UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */

