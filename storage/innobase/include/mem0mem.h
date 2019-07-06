/*****************************************************************************

Copyright (c) 1994, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/mem0mem.h
 The memory management

 Created 6/9/1994 Heikki Tuuri
 *******************************************************/

#ifndef mem0mem_h
#define mem0mem_h

#include <innodb/univ/univ.h>

#include <limits.h>
#include <innodb/allocator/mem_strdupl.h>
#include <innodb/memory/mem_block_t.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/memory/macros.h>
#include <innodb/memory/mem_block_validate.h>
#include <innodb/memory/mem_block_info_t.h>
#include <innodb/memory/mem_heap_validate.h>
#include <innodb/memory/mem_block_set_len.h>
#include <innodb/memory/mem_block_get_len.h>
#include <innodb/memory/mem_block_set_type.h>
#include <innodb/memory/mem_block_set_start.h>
#include <innodb/memory/mem_block_set_free.h>
#include <innodb/memory/mem_block_get_free.h>
#include <innodb/memory/mem_block_get_start.h>
#include <innodb/memory/mem_heap_get_size.h>
#include <innodb/memory/mem_heap_get_heap_top.h>
#include <innodb/memory/mem_heap_is_top.h>
#include <innodb/memory/validate_no_mans_land.h>
#include <innodb/memory/mem_heap_get_top.h>



/* -------------------- MEMORY HEAPS ----------------------------- */






#ifdef UNIV_DEBUG
/** Macro for memory heap creation.
@param[in]	size		Desired start block size. */
#define mem_heap_create(size) \
  mem_heap_create_func((size), __FILE__, __LINE__, MEM_HEAP_DYNAMIC)

/** Macro for memory heap creation.
@param[in]	size		Desired start block size.
@param[in]	type		Heap type */
#define mem_heap_create_typed(size, type) \
  mem_heap_create_func((size), __FILE__, __LINE__, (type))

#else /* UNIV_DEBUG */
/** Macro for memory heap creation.
@param[in]	size		Desired start block size. */
#define mem_heap_create(size) mem_heap_create_func((size), MEM_HEAP_DYNAMIC)

/** Macro for memory heap creation.
@param[in]	size		Desired start block size.
@param[in]	type		Heap type */
#define mem_heap_create_typed(size, type) mem_heap_create_func((size), (type))

#endif /* UNIV_DEBUG */

/** Creates a memory heap.
NOTE: Use the corresponding macros instead of this function.
A single user buffer of 'size' will fit in the block.
0 creates a default size block.
@param[in]	size		Desired start block size.
@param[in]	file_name	File name where created
@param[in]	line		Line where created
@param[in]	type		Heap type
@return own: memory heap, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
UNIV_INLINE
mem_heap_t *mem_heap_create_func(ulint size,
#ifdef UNIV_DEBUG
                                 const char *file_name, ulint line,
#endif /* UNIV_DEBUG */
                                 ulint type);

/** Frees the space occupied by a memory heap.
NOTE: Use the corresponding macro instead of this function.
@param[in]	heap	Heap to be freed */
UNIV_INLINE
void mem_heap_free(mem_heap_t *heap);

/** Allocates and zero-fills n bytes of memory from a memory heap.
@param[in]	heap	memory heap
@param[in]	n	number of bytes; if the heap is allowed to grow into
the buffer pool, this must be <= MEM_MAX_ALLOC_IN_BUF
@return allocated, zero-filled storage */
UNIV_INLINE
void *mem_heap_zalloc(mem_heap_t *heap, ulint n);

/** Allocates n bytes of memory from a memory heap.
@param[in]	heap	memory heap
@param[in]	n	number of bytes; if the heap is allowed to grow into
the buffer pool, this must be <= MEM_MAX_ALLOC_IN_BUF
@return allocated storage, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
UNIV_INLINE
void *mem_heap_alloc(mem_heap_t *heap, ulint n);


/** Frees the space in a memory heap exceeding the pointer given.
The pointer must have been acquired from mem_heap_get_heap_top.
The first memory block of the heap is not freed.
@param[in]	heap		heap from which to free
@param[in]	old_top		pointer to old top of heap */
UNIV_INLINE
void mem_heap_free_heap_top(mem_heap_t *heap, byte *old_top);

/** Empties a memory heap.
The first memory block of the heap is not freed.
@param[in]	heap		heap to empty */
UNIV_INLINE
void mem_heap_empty(mem_heap_t *heap);


/** Allocate a new chunk of memory from a memory heap, possibly discarding the
topmost element. If the memory chunk specified with (top, top_sz) is the
topmost element, then it will be discarded, otherwise it will be left untouched
and this function will be equivallent to mem_heap_alloc().
@param[in,out]	heap	memory heap
@param[in]	top	chunk to discard if possible
@param[in]	top_sz	size of top in bytes
@param[in]	new_sz	desired size of the new chunk
@return allocated storage, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
UNIV_INLINE
void *mem_heap_replace(mem_heap_t *heap, const void *top, ulint top_sz,
                       ulint new_sz);

/** Allocate a new chunk of memory from a memory heap, possibly discarding the
topmost element and then copy the specified data to it. If the memory chunk
specified with (top, top_sz) is the topmost element, then it will be discarded,
otherwise it will be left untouched and this function will be equivalent to
mem_heap_dup().
@param[in,out]	heap	memory heap
@param[in]	top	chunk to discard if possible
@param[in]	top_sz	size of top in bytes
@param[in]	data	new data to duplicate
@param[in]	data_sz	size of data in bytes
@return allocated storage, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
UNIV_INLINE
void *mem_heap_dup_replace(mem_heap_t *heap, const void *top, ulint top_sz,
                           const void *data, ulint data_sz);

/** Allocate a new chunk of memory from a memory heap, possibly discarding the
topmost element and then copy the specified string to it. If the memory chunk
specified with (top, top_sz) is the topmost element, then it will be discarded,
otherwise it will be left untouched and this function will be equivalent to
mem_heap_strdup().
@param[in,out]	heap	memory heap
@param[in]	top	chunk to discard if possible
@param[in]	top_sz	size of top in bytes
@param[in]	str	new data to duplicate
@return allocated string, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
UNIV_INLINE
char *mem_heap_strdup_replace(mem_heap_t *heap, const void *top, ulint top_sz,
                              const char *str);

/** Frees the topmost element in a memory heap.
@param[in]	heap	memory heap
@param[in]	n	size of the topmost element
The size of the element must be given. */
UNIV_INLINE
void mem_heap_free_top(mem_heap_t *heap, ulint n);


/** Duplicates a NUL-terminated string.
@param[in]	str	string to be copied
@return own: a copy of the string, must be deallocated with ut_free */
UNIV_INLINE
char *mem_strdup(const char *str);

/** Duplicates a NUL-terminated string, allocated from a memory heap.
@param[in]	heap	memory heap where string is allocated
@param[in]	str	string to be copied
@return own: a copy of the string */
char *mem_heap_strdup(mem_heap_t *heap, const char *str);

/** Makes a NUL-terminated copy of a nonterminated string, allocated from a
memory heap.
@param[in]	heap	memory heap where string is allocated
@param[in]	str	string to be copied
@param[in]	len	length of str, in bytes
@return own: a copy of the string */
UNIV_INLINE
char *mem_heap_strdupl(mem_heap_t *heap, const char *str, ulint len);

/** Concatenate two strings and return the result, using a memory heap.
 @return own: the result */
char *mem_heap_strcat(
    mem_heap_t *heap, /*!< in: memory heap where string is allocated */
    const char *s1,   /*!< in: string 1 */
    const char *s2);  /*!< in: string 2 */

/** Duplicate a block of data, allocated from a memory heap.
 @return own: a copy of the data */
void *mem_heap_dup(
    mem_heap_t *heap, /*!< in: memory heap where copy is allocated */
    const void *data, /*!< in: data to be copied */
    ulint len);       /*!< in: length of data, in bytes */

/** A simple sprintf replacement that dynamically allocates the space for the
 formatted string from the given heap. This supports a very limited set of
 the printf syntax: types 's' and 'u' and length modifier 'l' (which is
 required for the 'u' type).
 @return heap-allocated formatted string */
char *mem_heap_printf(mem_heap_t *heap,   /*!< in: memory heap */
                      const char *format, /*!< in: format string */
                      ...) MY_ATTRIBUTE((format(printf, 2, 3)));





/*#######################################################################*/
#include <innodb/lst/lst.h>



/* Header size for a memory heap block */
#define MEM_BLOCK_HEADER_SIZE \
  ut_calc_align(sizeof(mem_block_info_t), UNIV_MEM_ALIGNMENT)

#include "mem0mem.ic"

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

#endif
