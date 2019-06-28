#pragma once

#include <innodb/allocator/aligned_memory.h>
#include <innodb/counter/counter.h>

/** Manages an array of objects. The first element is aligned to specified
number of bytes.
@tparam	T_Type		type of the object that is going to be managed
@tparam T_Align_to	number of bytes to align to */
template <typename T_Type, size_t T_Align_to = INNOBASE_CACHE_LINE_SIZE>
class aligned_array_pointer : public aligned_memory<T_Type, T_Align_to> {
 public:
  /** Allocates aligned memory for new objects. Objects must be trivially
  constructible and destructible.
  @param[in]	size	Number of elements to allocate. */
  void create(size_t size) {
#if !(defined __GNUC__ && __GNUC__ <= 4)
    static_assert(std::is_trivially_default_constructible<T_Type>::value,
                  "Aligned array element type must be "
                  "trivially default-constructible");
#endif
    m_size = size;
    this->allocate(sizeof(T_Type) * m_size);
  }

  /** Deallocates memory of array created earlier. */
  void destroy() {
    static_assert(std::is_trivially_destructible<T_Type>::value,
                  "Aligned array element type must be "
                  "trivially destructible");
    this->free_memory();
  }

  /** Accesses specified index in the allocated array.
  @param[in]	index	index of element in array to get reference to */
  T_Type &operator[](size_t index) const {
    ut_a(index < m_size);
    return ((T_Type *)*this)[index];
  }

 private:
  /** Size of the allocated array. */
  size_t m_size;
};


