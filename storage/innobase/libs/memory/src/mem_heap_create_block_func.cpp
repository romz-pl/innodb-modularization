#include <innodb/memory/mem_heap_create_block_func.h>

#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/assert/assert.h>
#include <innodb/logger/fatal.h>
#include <innodb/memory/MEM_BLOCK_HEADER_SIZE.h>
#include <innodb/memory/macros.h>
#include <innodb/memory/mem_block_set_free.h>
#include <innodb/memory/mem_block_set_len.h>
#include <innodb/memory/mem_block_set_start.h>
#include <innodb/memory/mem_block_set_type.h>
#include <innodb/memory/mem_block_validate.h>
#include <innodb/memory/mem_heap_validate.h>
#include <innodb/memory_check/memory_check.h>

/* Reference to buffer pool - BEGIN */

struct buf_block_t;
struct buf_pool_t;
typedef byte buf_frame_t;

buf_block_t *buf_block_alloc(buf_pool_t *buf_pool);
buf_frame_t *buf_block_get_frame(const buf_block_t *block);

/* Reference to buffer pool - END */



/** Creates a memory heap block where data can be allocated.
 @return own: memory heap block, NULL if did not succeed (only possible
 for MEM_HEAP_BTR_SEARCH type heaps) */
mem_block_t *mem_heap_create_block_func(
    mem_heap_t *heap, /*!< in: memory heap or NULL if first block
                      should be created */
    ulint n,          /*!< in: number of bytes needed for user data */
#ifdef UNIV_DEBUG
    const char *file_name, /*!< in: file name where created */
    ulint line,            /*!< in: line where created */
#endif                     /* UNIV_DEBUG */
    ulint type)            /*!< in: type of heap: MEM_HEAP_DYNAMIC or
                           MEM_HEAP_BUFFER */
{
#ifndef UNIV_LIBRARY
  buf_block_t *buf_block = NULL;
#endif /* !UNIV_LIBRARY */
  mem_block_t *block;
  ulint len;

  ut_ad((type == MEM_HEAP_DYNAMIC) || (type == MEM_HEAP_BUFFER) ||
        (type == MEM_HEAP_BUFFER + MEM_HEAP_BTR_SEARCH));

  if (heap != NULL) {
    mem_block_validate(heap);
    ut_d(mem_heap_validate(heap));
  }

  /* In dynamic allocation, calculate the size: block header + data. */
  len = MEM_BLOCK_HEADER_SIZE + MEM_SPACE_NEEDED(n);

#if !defined(UNIV_LIBRARY) && !defined(UNIV_HOTBACKUP)
  if (type == MEM_HEAP_DYNAMIC || len < UNIV_PAGE_SIZE / 2) {
    ut_ad(type == MEM_HEAP_DYNAMIC || n <= MEM_MAX_ALLOC_IN_BUF);

    block = static_cast<mem_block_t *>(ut_malloc_nokey(len));
  } else {
    len = UNIV_PAGE_SIZE;

    if ((type & MEM_HEAP_BTR_SEARCH) && heap) {
      /* We cannot allocate the block from the
      buffer pool, but must get the free block from
      the heap header free block field */

      buf_block = static_cast<buf_block_t *>(heap->free_block);
      heap->free_block = NULL;

      if (UNIV_UNLIKELY(!buf_block)) {
        return (NULL);
      }
    } else {
      buf_block = buf_block_alloc(NULL);
    }

    // block = (mem_block_t *)buf_block->frame;
    block = (mem_block_t *)buf_block_get_frame(buf_block);
  }

  if (block == NULL) {
#ifdef UNIV_NO_ERR_MSGS
    ib::fatal()
#else
    ib::fatal(ER_IB_MSG_1274)
#endif /* !UNIV_NO_ERR_MSGS */
        << "Unable to allocate memory of size " << len << ".";
  }

  /* Make only the header part of the block accessible. If it is a block
  from the buffer pool, the len will be UNIV_PAGE_SIZE already. */
  UNIV_MEM_FREE(block, len);
  UNIV_MEM_ALLOC(block, MEM_BLOCK_HEADER_SIZE);

  block->buf_block = buf_block;
  block->free_block = NULL;

#else  /* !UNIV_LIBRARY && !UNIV_HOTBACKUP */
  len = MEM_BLOCK_HEADER_SIZE + MEM_SPACE_NEEDED(n);
  block = static_cast<mem_block_t *>(ut_malloc_nokey(len));
  ut_a(block);
  block->free_block = NULL;
#endif /* !UNIV_LIBRARY && !UNIV_HOTBACKUP */

  ut_d(ut_strlcpy_rev(block->file_name, file_name, sizeof(block->file_name)));
  ut_d(block->line = line);

  block->magic_n = MEM_BLOCK_MAGIC_N;
  mem_block_set_len(block, len);
  mem_block_set_type(block, type);
  mem_block_set_start(block, MEM_BLOCK_HEADER_SIZE);
  mem_block_set_free(block, MEM_BLOCK_HEADER_SIZE);

  if (UNIV_UNLIKELY(heap == NULL)) {
    /* This is the first block of the heap. The field
    total_size should be initialized here */
    block->total_size = len;
  } else {
    /* Not the first allocation for the heap. This block's
    total_length field should be set to undefined and never
    actually used. */
    ut_d(block->total_size = ULINT_UNDEFINED);
    UNIV_MEM_FREE(&block->total_size, sizeof block->total_size);

    heap->total_size += len;
  }

  ut_ad((ulint)MEM_BLOCK_HEADER_SIZE < len);

  return (block);
}



