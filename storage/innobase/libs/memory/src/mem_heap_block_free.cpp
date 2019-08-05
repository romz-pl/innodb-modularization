#include <innodb/memory/mem_heap_block_free.h>

#include <innodb/memory/mem_block_validate.h>
#include <innodb/assert/assert.h>
#include <innodb/memory/macros.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/memory_check/memory_check.h>

struct buf_block_t;
void buf_block_free(buf_block_t *block);

/** Frees a block from a memory heap. */
void mem_heap_block_free(mem_heap_t *heap,   /*!< in: heap */
                         mem_block_t *block) /*!< in: block to free */
{
#ifndef UNIV_LIBRARY
  buf_block_t *buf_block;

  buf_block = static_cast<buf_block_t *>(block->buf_block);
#endif /* !UNIV_LIBRARY */

  mem_block_validate(block);

  UT_LIST_REMOVE(heap->base, block);

  ut_ad(heap->total_size >= block->len);
  heap->total_size -= block->len;

#ifndef UNIV_LIBRARY
  ulint type = heap->type;
  ulint len = block->len;
#endif /* !UNIV_LIBRARY */

  block->magic_n = MEM_FREED_BLOCK_MAGIC_N;

#ifdef UNIV_DEBUG
  if (mem_block_get_start(block) != mem_block_get_free(block)) {
    validate_no_mans_land((byte *)block + mem_block_get_start(block),
                          MEM_NO_MANS_LAND_BEFORE_BYTE);
    validate_no_mans_land(
        (byte *)block + mem_block_get_free(block) - MEM_NO_MANS_LAND,
        MEM_NO_MANS_LAND_AFTER_BYTE);
  }
#endif

#if !defined(UNIV_LIBRARY) && !defined(UNIV_HOTBACKUP)
  if (type == MEM_HEAP_DYNAMIC || len < UNIV_PAGE_SIZE / 2) {
    ut_ad(!buf_block);
    ut_free(block);
  } else {
    ut_ad(type & MEM_HEAP_BUFFER);

    /* Make memory available again for buffer pool, as we set parts
    of block to "free" state in heap allocator. */
    UNIV_MEM_ALLOC(block, UNIV_PAGE_SIZE);
    buf_block_free(buf_block);
  }
#else  /* !UNIV_LIBRARY && !UNIV_HOTBACKUP */
  ut_free(block);
#endif /* !UNIV_LIBRARY && !UNIV_HOTBACKUP */
}
