#pragma once

struct buf_block_t;
struct buf_chunk_t;

const buf_block_t *buf_chunk_not_freed(
    buf_chunk_t *chunk); /*!< in: chunk being checked */
