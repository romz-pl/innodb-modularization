#pragma once

#include <innodb/univ/univ.h>

#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/assert/assert.h>

/** Abstract class to manage an object that is aligned to specified number of
bytes.
@tparam	T_Type		type of the object that is going to be managed
@tparam T_Align_to	number of bytes to align to */
template <typename T_Type, size_t T_Align_to>
class aligned_memory {
 public:
  virtual ~aligned_memory() {
    if (!this->is_object_empty()) {
      this->free_memory();
    }
  }

  virtual void destroy() = 0;

  /** Allows casting to managed objects type to use it directly */
  operator T_Type *() const {
    ut_a(m_object != nullptr);
    return m_object;
  }

  /** Allows referencing the managed object as this was a normal
  pointer. */
  T_Type *operator->() const {
    ut_a(m_object != nullptr);
    return m_object;
  }

 protected:
  /** Checks if no object is currently being managed. */
  bool is_object_empty() const { return m_object == nullptr; }

  /** Allocates memory for a new object and prepares aligned address for
  the object.
  @param[in]	size	Number of bytes to be delivered for the aligned
  object. Number of bytes actually allocated will be higher. */
  T_Type *allocate(size_t size) {
    static_assert(T_Align_to > 0, "Incorrect alignment parameter");
    ut_a(m_memory == nullptr);
    ut_a(m_object == nullptr);

    m_memory = ut_zalloc_nokey(size + T_Align_to - 1);
    m_object = static_cast<T_Type *>(::ut_align(m_memory, T_Align_to));
    return m_object;
  }

  /** Releases memory used to store the object. */
  void free_memory() {
    ut_a(m_memory != nullptr);
    ut_a(m_object != nullptr);

    ut_free(m_memory);

    m_memory = nullptr;
    m_object = nullptr;
  }

 private:
  /** Stores pointer to aligned object memory. */
  T_Type *m_object = nullptr;

  /** Stores pointer to memory used to allocate the object. */
  void *m_memory = nullptr;
};
