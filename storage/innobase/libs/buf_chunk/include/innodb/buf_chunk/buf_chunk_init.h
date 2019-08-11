#pragma once

#include <innodb/univ/univ.h>

#include <mutex>

struct buf_pool_t;
struct buf_chunk_t;

buf_chunk_t *buf_chunk_init(
    buf_pool_t *buf_pool, /*!< in: buffer pool instance */
    buf_chunk_t *chunk,   /*!< out: chunk of buffers */
    ulonglong mem_size,   /*!< in: requested size in bytes */
    std::mutex *mutex);    /*!< in,out: Mutex protecting chunk map. */
