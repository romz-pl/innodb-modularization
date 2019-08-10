#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Chnunk_Bitmap.h>
#include <innodb/clone/Chunk_Map.h>


/** Incomplete Chunk information */
struct Chunk_Info {
  /** Information about chunks completed */
  Chnunk_Bitmap m_reserved_chunks;

  /** Information about unfinished chunks */
  Chunk_Map m_incomplete_chunks;

  /** Chunks for current state */
  uint32_t m_total_chunks;

  /** Minimum chunk number that is not reserved yet */
  uint32_t m_min_unres_chunk;

  /** Maximum chunk number that is already reserved */
  uint32_t m_max_res_chunk;

  /** Initialize Chunk number ranges */
  void init_chunk_nums() {
    m_min_unres_chunk = m_reserved_chunks.get_min_unset_bit();
    ut_ad(m_min_unres_chunk <= m_total_chunks + 1);

    m_max_res_chunk = m_reserved_chunks.get_max_set_bit();
    ut_ad(m_max_res_chunk <= m_total_chunks);
  }

  /** Serialize the descriptor. Caller should pass
  the length if allocated.
  @param[out]	desc_chunk	serialized chunk info
  @param[in,out]	len		length of serialized descriptor */
  void serialize(byte *desc_chunk, uint &len);

  /** Deserialize the descriptor.
  @param[in]	desc_chunk	serialized chunk info
  @param[in,out]	len_left	length left in bytes */
  void deserialize(const byte *desc_chunk, uint &len_left);

  /** Get the length of serialized data
  @param[in]	num_tasks	number of tasks to include
  @return length serialized chunk info */
  size_t get_serialized_length(uint32_t num_tasks);
};
