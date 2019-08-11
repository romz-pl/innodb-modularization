#include <innodb/buf_chunk/buf_get_nth_chunk_block.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_block/buf_block_t.h>
#include <innodb/buf_pool/buf_pool_t.h>
#include <innodb/buf_chunk/buf_chunk_t.h>

/** Get the nth chunk's buffer block in the specified buffer pool.
 @return the nth chunk's buffer block. */
buf_block_t *buf_get_nth_chunk_block(
    const buf_pool_t *buf_pool, /*!< in: buffer pool instance */
    ulint n,                    /*!< in: nth chunk in the buffer pool */
    ulint *chunk_size)          /*!< in: chunk size */
{
  const buf_chunk_t *chunk;

  chunk = buf_pool->chunks + n;
  *chunk_size = chunk->size;
  return (chunk->blocks);
}

#endif
