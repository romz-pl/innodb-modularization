#include <innodb/clone/Chunk_Info.h>

#include <innodb/machine/data.h>
#include <innodb/clone/flags.h>

size_t Chunk_Info::get_serialized_length(uint32_t num_tasks) {
  /* Length of incomplete chunk data */
  size_t ret_size = 4;

  auto num_elements = m_incomplete_chunks.size();

  /* Have bigger allocated length if requested */
  if (num_tasks > num_elements) {
    num_elements = num_tasks;
  }

  /* Add size for incomplete chunks data. Serialized element
  has chunk and block number: 4 + 4 = 8 bytes */
  ret_size += (8 * num_elements);

  /* Add length of chunk bitmap data */
  ret_size += m_reserved_chunks.get_serialized_length();

  return (ret_size);
}


void Chunk_Info::serialize(byte *desc_chunk, uint &len) {
  auto len_left = len;
  auto chunk_map_size = static_cast<ulint>(m_incomplete_chunks.size());

  mach_write_to_4(desc_chunk, chunk_map_size);
  desc_chunk += 4;

  ut_ad(len_left >= 4);
  len_left -= 4;

  ulint index = 0;

  for (auto &key_value : m_incomplete_chunks) {
    ut_ad(index < chunk_map_size);

    mach_write_to_4(desc_chunk, key_value.first);
    desc_chunk += 4;

    ut_ad(len_left >= 4);
    len_left -= 4;

    mach_write_to_4(desc_chunk, key_value.second);
    desc_chunk += 4;

    ut_ad(len_left >= 4);
    len_left -= 4;

    ++index;
  }
  ut_ad(index == chunk_map_size);

  /* Actual length for serialized chunk map */
  ut_ad(len > len_left);
  len -= len_left;

  m_reserved_chunks.serialize(desc_chunk, len_left);

  /* Total serialized length */
  len += len_left;
}

void Chunk_Info::deserialize(const byte *desc_chunk, uint &len_left) {
  auto chunk_map_size = mach_read_from_4(desc_chunk);

  desc_chunk += 4;

  if (len_left < 4) {
    ut_ad(false);
    return;
  }

  len_left -= 4;

  /* Each task can have one incomplete chunk at most */
  if (chunk_map_size > CLONE_MAX_TASKS) {
    ut_ad(false);
    return;
  }

  for (ulint index = 0; index < chunk_map_size; index++) {
    auto chunk_num = static_cast<uint32_t>(mach_read_from_4(desc_chunk));

    desc_chunk += 4;

    if (len_left < 4) {
      ut_ad(false);
      return;
    }
    len_left -= 4;

    auto block_num = static_cast<uint32_t>(mach_read_from_4(desc_chunk));
    desc_chunk += 4;

    if (len_left < 4) {
      ut_ad(false);
      return;
    }
    len_left -= 4;

    m_incomplete_chunks[chunk_num] = block_num;
  }

  m_reserved_chunks.deserialize(desc_chunk, len_left);

  ut_ad(len_left == 0);
}
