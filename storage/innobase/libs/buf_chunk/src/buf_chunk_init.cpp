#include <innodb/buf_chunk/buf_chunk_init.h>

#include <innodb/buf_pool/buf_pool_t.h>
#include <innodb/buf_chunk/buf_chunk_t.h>
#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/math/ut_2pow_round.h>
#include <innodb/align/ut_align.h>
#include <innodb/buf_block/buf_block_init.h>
#include <innodb/buf_pool/buf_pool_register_chunk.h>
#include <innodb/memory_check/memory_check.h>

/** Allocates a chunk of buffer frames. If called for an existing buf_pool, its
 free_list_mutex must be locked.
 @return chunk, or NULL on failure */
buf_chunk_t *buf_chunk_init(
    buf_pool_t *buf_pool, /*!< in: buffer pool instance */
    buf_chunk_t *chunk,   /*!< out: chunk of buffers */
    ulonglong mem_size,   /*!< in: requested size in bytes */
    std::mutex *mutex)    /*!< in,out: Mutex protecting chunk map. */
{
  buf_block_t *block;
  byte *frame;
  ulint i;

  mutex_own(&buf_pool->chunks_mutex);

  /* Round down to a multiple of page size,
  although it already should be. */
  mem_size = ut_2pow_round(mem_size, UNIV_PAGE_SIZE);
  /* Reserve space for the block descriptors. */
  mem_size += ut_2pow_round(
      (mem_size / UNIV_PAGE_SIZE) * (sizeof *block) + (UNIV_PAGE_SIZE - 1),
      UNIV_PAGE_SIZE);

  DBUG_EXECUTE_IF("ib_buf_chunk_init_fails", return (NULL););

  if (!buf_pool->allocate_chunk(mem_size, chunk)) {
    return (NULL);
  }

#ifdef HAVE_LIBNUMA
  if (srv_numa_interleave) {
    int st = mbind(chunk->mem, chunk->mem_size(), MPOL_INTERLEAVE,
                   numa_all_nodes_ptr->maskp, numa_all_nodes_ptr->size,
                   MPOL_MF_MOVE);
    if (st != 0) {
      ib::warn(ER_IB_MSG_54) << "Failed to set NUMA memory policy of"
                                " buffer pool page frames to MPOL_INTERLEAVE"
                                " (error: "
                             << strerror(errno) << ").";
    }
  }
#endif /* HAVE_LIBNUMA */

  /* Allocate the block descriptors from
  the start of the memory block. */
  chunk->blocks = (buf_block_t *)chunk->mem;

  /* Align a pointer to the first frame.  Note that when
  os_large_page_size is smaller than UNIV_PAGE_SIZE,
  we may allocate one fewer block than requested.  When
  it is bigger, we may allocate more blocks than requested. */

  frame = (byte *)ut_align(chunk->mem, UNIV_PAGE_SIZE);
  chunk->size = chunk->mem_pfx.m_size / UNIV_PAGE_SIZE - (frame != chunk->mem);

  /* Subtract the space needed for block descriptors. */
  {
    ulint size = chunk->size;

    while (frame < (byte *)(chunk->blocks + size)) {
      frame += UNIV_PAGE_SIZE;
      size--;
    }

    chunk->size = size;
  }

  /* Init block structs and assign frames for them. Then we
  assign the frames to the first blocks (we already mapped the
  memory above). */

  block = chunk->blocks;

  for (i = chunk->size; i--;) {
    buf_block_init(buf_pool, block, frame);
    UNIV_MEM_INVALID(block->frame, UNIV_PAGE_SIZE);

    /* Add the block to the free list */
    UT_LIST_ADD_LAST(buf_pool->free, &block->page);

    ut_d(block->page.in_free_list = TRUE);
    ut_ad(buf_pool_from_block(block) == buf_pool);

    block++;
    frame += UNIV_PAGE_SIZE;
  }

  if (mutex != nullptr) {
    mutex->lock();
  }

  buf_pool_register_chunk(chunk);

  if (mutex != nullptr) {
    mutex->unlock();
  }

#ifdef PFS_GROUP_BUFFER_SYNC
  pfs_register_buffer_block(chunk);
#endif /* PFS_GROUP_BUFFER_SYNC */
  return (chunk);
}
