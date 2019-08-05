#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_alloc.h>

/** A C++ wrapper class to the mem_heap_t routines, so that it can be used
as an STL allocator */
template <typename T>
class mem_heap_allocator {
 public:
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T *pointer;
  typedef const T *const_pointer;
  typedef T &reference;
  typedef const T &const_reference;

  mem_heap_allocator(mem_heap_t *heap) : m_heap(heap) {}

  mem_heap_allocator(const mem_heap_allocator &other) : m_heap(other.m_heap) {
    // Do nothing
  }

  template <typename U>
  mem_heap_allocator(const mem_heap_allocator<U> &other)
      : m_heap(other.m_heap) {
    // Do nothing
  }

  ~mem_heap_allocator() { m_heap = 0; }

  size_type max_size() const { return (ULONG_MAX / sizeof(T)); }

  /** This function returns a pointer to the first element of a newly
  allocated array large enough to contain n objects of type T; only the
  memory is allocated, and the objects are not constructed. Moreover,
  an optional pointer argument (that points to an object already
  allocated by mem_heap_allocator) can be used as a hint to the
  implementation about where the new memory should be allocated in
  order to improve locality. */
  pointer allocate(size_type n, const_pointer hint = 0) {
    return (reinterpret_cast<pointer>(mem_heap_alloc(m_heap, n * sizeof(T))));
  }

  void deallocate(pointer p, size_type n) {}

  pointer address(reference r) const { return (&r); }

  const_pointer address(const_reference r) const { return (&r); }

  void construct(pointer p, const_reference t) {
    new (reinterpret_cast<void *>(p)) T(t);
  }

  void destroy(pointer p) { (reinterpret_cast<T *>(p))->~T(); }

  /** Allocators are required to supply the below template class member
  which enables the possibility of obtaining a related allocator,
  parametrized in terms of a different type. For example, given an
  allocator type IntAllocator for objects of type int, a related
  allocator type for objects of type long could be obtained using
  IntAllocator::rebind<long>::other */
  template <typename U>
  struct rebind {
    typedef mem_heap_allocator<U> other;
  };

  /** Get the underlying memory heap object.
  @return the underlying memory heap object. */
  mem_heap_t *get_mem_heap() const { return (m_heap); }

 private:
  mem_heap_t *m_heap;
  template <typename U>
  friend class mem_heap_allocator;
};

template <class T>
bool operator==(const mem_heap_allocator<T> &left,
                const mem_heap_allocator<T> &right) {
  return (left.get_mem_heap() == right.get_mem_heap());
}

template <class T>
bool operator!=(const mem_heap_allocator<T> &left,
                const mem_heap_allocator<T> &right) {
  return (left.get_mem_heap() != right.get_mem_heap());
}
