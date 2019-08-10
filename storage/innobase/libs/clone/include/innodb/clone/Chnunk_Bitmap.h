#pragma once

#include <innodb/univ/univ.h>

#include <innodb/assert/assert.h>
#include <innodb/memory/mem_heap_t.h>

/** Bitmap for completed chunks in current state */
class Chnunk_Bitmap {
 public:
  /** Construct bitmap */
  Chnunk_Bitmap() : m_bitmap(), m_size(), m_bits() {}

  /** Bitmap array index operator implementation */
  class Bitmap_Operator_Impl {
   public:
    /** Construct bitmap operator
    @param[in]	bitmap	reference to bitmap buffer
    @param[in]	index	array operation index */
    Bitmap_Operator_Impl(uint32_t *&bitmap, uint32_t index)

        : m_bitmap_ref(bitmap) {
      /* BYTE position */
      auto byte_index = index >> 3;
      ut_ad(byte_index == index / 8);

      /* MAP array position */
      m_map_index = byte_index >> 2;
      ut_ad(m_map_index == byte_index / 4);

      /* BIT position */
      auto bit_pos = index & 31;
      ut_ad(bit_pos == index % 32);

      m_bit_mask = 1 << bit_pos;
    }

    /** Check value at specified index in BITMAP
    @return true if the BIT is set */
    operator bool() const {
      auto &val = m_bitmap_ref[m_map_index];

      if ((val & m_bit_mask) == 0) {
        return (false);
      }

      return (true);
    }

    /** Set BIT at specific index
    @param[in]	bit	bit value to set */
    void operator=(bool bit) {
      auto &val = m_bitmap_ref[m_map_index];

      if (bit) {
        val |= m_bit_mask;
      } else {
        val &= ~m_bit_mask;
      }
    }

   private:
    /** Refernce to BITMAP array */
    uint32_t *&m_bitmap_ref;

    /** Current array position */
    uint32_t m_map_index;

    /** Mask with current BIT set */
    uint32_t m_bit_mask;
  };

  /** Array index operator
  @param[in]	index	bitmap array index
  @return	operator implementation object */
  Bitmap_Operator_Impl operator[](uint32_t index) {
    /* Convert to zero based index */
    --index;

    ut_a(index < m_bits);
    return (Bitmap_Operator_Impl(m_bitmap, index));
  }

  /** Reset bitmap with new size
  @param[in]	max_bits	number of BITs to hold
  @param[in]	heap		heap for allocating memory
  @return	old buffer pointer */
  uint32_t *reset(uint32_t max_bits, mem_heap_t *heap);

  /** Get minimum BIT position that is not set
  @return BIT position */
  uint32_t get_min_unset_bit();

  /** Get maximum BIT position that is not set
  @return BIT position */
  uint32_t get_max_set_bit();

  /** Serialize the descriptor. Caller should pass
  the length if allocated.
  @param[out]	desc_chunk	serialized chunk info
  @param[in,out]	len		length of serialized descriptor */
  void serialize(byte *&desc_chunk, uint &len);

  /** Deserialize the descriptor.
  @param[in]	desc_chunk	serialized chunk info
  @param[in,out]	len_left	length left in bytes */
  void deserialize(const byte *desc_chunk, uint &len_left);

  /** Get the length of serialized data
  @return length serialized chunk info */
  size_t get_serialized_length();

  /** Maximum bit capacity
  @return maximum number of BITs it can hold */
  size_t capacity() const { return (8 * size()); }

  /** Size of bitmap in bytes
  @return BITMAP buffer size */
  size_t size() const { return (m_size * 4); }

  /** Size of bitmap in bits
  @return number of BITs stored */
  uint32_t size_bits() const { return (m_bits); }

 private:
  /** BITMAP buffer */
  uint32_t *m_bitmap;

  /** BITMAP buffer size: Number of 4 byte blocks */
  size_t m_size;

  /** Total number of BITs in the MAP */
  uint32_t m_bits;
};
