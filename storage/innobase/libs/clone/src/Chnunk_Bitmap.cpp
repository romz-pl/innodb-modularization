#include <innodb/clone/Chnunk_Bitmap.h>

#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/machine/data.h>

uint32_t *Chnunk_Bitmap::reset(uint32_t max_bits, mem_heap_t *heap) {
  m_bits = max_bits;

  if (max_bits <= capacity()) {
    if (m_bitmap != nullptr && size() > 0) {
      memset(m_bitmap, 0, size());
    }
    return (nullptr);
  }

  auto old_buf = m_bitmap;

  m_size = static_cast<size_t>(m_bits >> 3);

  ut_ad(m_size == m_bits / 8);

  if (max_bits > capacity()) {
    ++m_size;
  }

  ut_ad(m_bits <= capacity());

  m_bitmap = static_cast<uint32_t *>(
      mem_heap_zalloc(heap, static_cast<ulint>(size())));

  return (old_buf);
}

uint32_t Chnunk_Bitmap::get_min_unset_bit() {
  uint32_t mask = 0;
  uint32_t return_bit = 0;
  size_t index = 0;

  mask = ~mask;

  /* Find the first block with unset BIT */
  for (index = 0; index < m_size; ++index) {
    if ((m_bitmap[index] & mask) != mask || return_bit >= m_bits) {
      break;
    }

    return_bit += 32;
  }

  /* All BITs are set */
  if (index >= m_size || return_bit >= m_bits) {
    return (m_bits + 1);
  }

  auto val = m_bitmap[index];
  ut_ad((val & mask) != mask);

  index = 0;

  /* Find the unset BIT within block */
  do {
    mask = 1 << index;

    if ((val & mask) == 0) {
      break;
    }

  } while (++index < 32);

  ut_ad(index < 32);

  return_bit += static_cast<uint32_t>(index);

  /* Change from 0 to 1 based index */
  ++return_bit;

  ut_ad(return_bit <= m_bits + 1);

  return (return_bit);
}

uint32_t Chnunk_Bitmap::get_max_set_bit() {
  uint32_t return_bit = 0;
  size_t block_index = 0;
  size_t index = 0;

  /* Find the last block with set BIT */
  for (index = 0; index < m_size; ++index) {
    if (return_bit >= m_bits) {
      break;
    }

    if (m_bitmap[index] != 0) {
      block_index = index + 1;
    }

    return_bit += 32;
  }

  /* No BITs are set */
  if (block_index == 0) {
    return (0);
  }

  --block_index;
  return_bit = static_cast<uint32_t>(block_index * 32);

  auto val = m_bitmap[block_index];
  ut_ad(val != 0);

  uint32_t mask = 0;
  index = 0;

  /* Find the last BIT set within block */
  do {
    mask = 1 << index;

    if ((val & mask) != 0) {
      block_index = index;
    }

  } while (++index < 32);

  return_bit += static_cast<uint32_t>(block_index);

  /* Change from 0 to 1 based index */
  ++return_bit;

  ut_ad(return_bit <= m_bits);

  return (return_bit);
}

size_t Chnunk_Bitmap::get_serialized_length() {
  /* Length of chunk BITMAP data */
  size_t ret_size = 4;

  /* Add size for chunk bitmap data */
  ret_size += size();

  return (ret_size);
}


void Chnunk_Bitmap::serialize(byte *&desc_chunk, uint &len) {
  auto len_left = len;
  auto bitmap_size = static_cast<ulint>(m_size);

  mach_write_to_4(desc_chunk, bitmap_size);
  desc_chunk += 4;

  ut_ad(len_left >= 4);
  len_left -= 4;

  for (size_t index = 0; index < m_size; ++index) {
    auto val = static_cast<ulint>(m_bitmap[index]);

    mach_write_to_4(desc_chunk, val);
    desc_chunk += 4;

    ut_ad(len_left >= 4);
    len_left -= 4;
  }

  ut_ad(len > len_left);
  len -= len_left;
}


void Chnunk_Bitmap::deserialize(const byte *desc_chunk, uint &len_left) {
  auto bitmap_size = mach_read_from_4(desc_chunk);
  desc_chunk += 4;

  if (len_left < 4) {
    ut_ad(false);
    return;
  }

  len_left -= 4;

  if (bitmap_size > m_size) {
    ut_ad(false);
    return;
  }

  for (ulint index = 0; index < bitmap_size; index++) {
    m_bitmap[index] = static_cast<uint32_t>(mach_read_from_4(desc_chunk));

    desc_chunk += 4;

    if (len_left < 4) {
      ut_ad(false);
      return;
    }

    len_left -= 4;
  }

  ut_ad(len_left == 0);
}
