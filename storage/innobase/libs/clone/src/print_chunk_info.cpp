#include <innodb/clone/print_chunk_info.h>

#include <innodb/clone/Chunk_Info.h>
#include <innodb/logger/info.h>


/** Print completed chunk information
@param[in]	chunk_info	chunk information */
void print_chunk_info(Chunk_Info *chunk_info) {
  for (auto &chunk : chunk_info->m_incomplete_chunks) {
    ib::info(ER_IB_MSG_151)
        << "Incomplete: Chunk = " << chunk.first << " Block = " << chunk.second;
  }

  auto min = chunk_info->m_reserved_chunks.get_min_unset_bit();
  auto max = chunk_info->m_reserved_chunks.get_max_set_bit();

  auto size = chunk_info->m_reserved_chunks.size_bits();

  ib::info(ER_IB_MSG_151) << "Number of Chunks: " << size << " Min = " << min
                          << " Max = " << max;

  ut_ad(min != max);

  if (max > min) {
    ib::info(ER_IB_MSG_151) << "Reserved Chunk Information : " << min << " - "
                            << max << " Chunks: " << max - min + 1;

    for (uint32_t index = min; index <= max;) {
      uint32_t ind = 0;

      const int STR_SIZE = 64;
      char str[STR_SIZE + 1];

      while (index <= max && ind < STR_SIZE) {
        str[ind] = chunk_info->m_reserved_chunks[index] ? '1' : '0';
        ++index;
        ++ind;
      }

      ut_ad(ind <= STR_SIZE);
      str[ind] = '\0';

      ib::info(ER_IB_MSG_151) << str;
    }
  }
}

