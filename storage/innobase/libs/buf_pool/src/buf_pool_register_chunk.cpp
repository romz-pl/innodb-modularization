#include <innodb/buf_pool/buf_pool_register_chunk.h>

#include <innodb/buf_chunk/buf_pool_chunk_map_t.h>
#include <innodb/buf_chunk/buf_chunk_t.h>
#include <innodb/buf_chunk/buf_chunk_map_reg.h>
#include <innodb/buf_block/buf_block_t.h>

/** Registers a chunk to buf_pool_chunk_map
@param[in]	chunk	chunk of buffers */
void buf_pool_register_chunk(buf_chunk_t *chunk) {
  buf_chunk_map_reg->insert(
      buf_pool_chunk_map_t::value_type(chunk->blocks->frame, chunk));
}
