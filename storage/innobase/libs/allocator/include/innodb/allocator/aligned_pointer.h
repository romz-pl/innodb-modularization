#pragma once

#include <innodb/allocator/aligned_memory.h>
#include <innodb/counter/counter.h>

/** Manages an object that is aligned to specified number of bytes.
@tparam	T_Type		type of the object that is going to be managed
@tparam T_Align_to	number of bytes to align to */
template <typename T_Type, size_t T_Align_to = INNOBASE_CACHE_LINE_SIZE>
class aligned_pointer : public aligned_memory<T_Type, T_Align_to> {
 public:
  ~aligned_pointer() {
    if (!this->is_object_empty()) {
      this->destroy();
    }
  }

  /** Allocates aligned memory for new object and constructs it there.
  @param[in]	args	arguments to be passed to object constructor */
  template <typename... T_Args>
  void create(T_Args... args) {
    new (this->allocate(sizeof(T_Type))) T_Type(std::forward(args)...);
  }

  /** Destroys the managed object and releases its memory. */
  void destroy() {
    (*this)->~T_Type();
    this->free_memory();
  }
};
