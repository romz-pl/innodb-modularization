#pragma once

#include <innodb/univ/univ.h>

#include <cstring>
#include <limits>

#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/psi_base.h"

#include <innodb/allocator/alloc_max_retries.h>
#include <innodb/allocator/OUT_OF_MEMORY_MSG.h>
#include <innodb/allocator/os_mem_alloc_large.h>
#include <innodb/allocator/os_mem_free_large.h>
#include <innodb/allocator/psi.h>
#include <innodb/allocator/ut_new_pfx_t.h>
#include <innodb/assert/assert.h>
#include <innodb/logger/fatal_or_error.h>
#include <innodb/thread/os_thread_sleep.h>



/** Allocator class for allocating memory from inside std::* containers. */
template <class T>
class ut_allocator {
 public:
  typedef T *pointer;
  typedef const T *const_pointer;
  typedef T &reference;
  typedef const T &const_reference;
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  /** Default constructor.
  @param[in] key  performance schema key. */
  explicit ut_allocator(PSI_memory_key key = PSI_NOT_INSTRUMENTED)
      :
#ifdef UNIV_PFS_MEMORY
        m_key(key),
#endif /* UNIV_PFS_MEMORY */
        m_oom_fatal(true) {
  }

  /** Constructor from allocator of another type.
  @param[in] other  the allocator to copy. */
  template <class U>
  ut_allocator(const ut_allocator<U> &other)
      :
#ifdef UNIV_PFS_MEMORY
        m_key(other.get_mem_key()),
#endif /* UNIV_PFS_MEMORY */
        m_oom_fatal(other.is_oom_fatal()) {
  }

  /** When out of memory (OOM) happens, report error and do not
  make it fatal.
  @return a reference to the allocator. */
  ut_allocator &set_oom_not_fatal() {
    m_oom_fatal = false;
    return (*this);
  }

  /** Check if allocation failure is a fatal error.
  @return true if allocation failure is fatal, false otherwise. */
  bool is_oom_fatal() const { return (m_oom_fatal); }

#ifdef UNIV_PFS_MEMORY
  /** Get the performance schema key to use for tracing allocations.
  @return performance schema key */
  PSI_memory_key get_mem_key() const {
    /* note: keep this as simple getter as is used by copy constructor */
    return (m_key);
  }
#endif /* UNIV_PFS_MEMORY */

  /** Return the maximum number of objects that can be allocated by
  this allocator. */
  size_type max_size() const {
    const size_type s_max = std::numeric_limits<size_type>::max();

#ifdef UNIV_PFS_MEMORY
    return ((s_max - sizeof(ut_new_pfx_t)) / sizeof(T));
#else
    return (s_max / sizeof(T));
#endif /* UNIV_PFS_MEMORY */
  }

  /** Allocate a chunk of memory that can hold 'n_elements' objects of
  type 'T' and trace the allocation.
  If the allocation fails this method may throw an exception. This
  is mandated by the standard and if it returns NULL instead, then
  STL containers that use it (e.g. std::vector) may get confused.
  After successful allocation the returned pointer must be passed
  to ut_allocator::deallocate() when no longer needed.
  @param[in]  n_elements      number of elements
  @param[in]  hint            pointer to a nearby memory location,
                              unused by this implementation
  @param[in]  key             performance schema key
  @param[in]  set_to_zero     if true, then the returned memory is
                              initialized with 0x0 bytes.
  @param[in]  throw_on_error  if true, then exception is throw on
                              allocation failure
  @return pointer to the allocated memory */
  pointer allocate(size_type n_elements, const_pointer hint = NULL,
                   PSI_memory_key key = PSI_NOT_INSTRUMENTED,
                   bool set_to_zero = false, bool throw_on_error = true) {
    if (n_elements == 0) {
      return (NULL);
    }

    if (n_elements > max_size()) {
      if (throw_on_error) {
        throw(std::bad_alloc());
      } else {
        return (NULL);
      }
    }

    void *ptr;
    size_t total_bytes = n_elements * sizeof(T);

#ifdef UNIV_PFS_MEMORY
    /* The header size must not ruin the 64-bit alignment
    on 32-bit systems. Some allocated structures use
    64-bit fields. */
    ut_ad((sizeof(ut_new_pfx_t) & 7) == 0);
    total_bytes += sizeof(ut_new_pfx_t);
#endif /* UNIV_PFS_MEMORY */

    for (size_t retries = 1;; retries++) {
      if (set_to_zero) {
        ptr = calloc(1, total_bytes);
      } else {
        ptr = malloc(total_bytes);
      }

      if (ptr != NULL || retries >= alloc_max_retries) {
        break;
      }

      os_thread_sleep(1000000 /* 1 second */);
    }

    if (ptr == NULL) {
      ib::fatal_or_error(m_oom_fatal)
          << "Cannot allocate " << total_bytes << " bytes of memory after "
          << alloc_max_retries << " retries over " << alloc_max_retries
          << " seconds. OS error: " << strerror(errno) << " (" << errno << "). "
          << OUT_OF_MEMORY_MSG;
      if (throw_on_error) {
        throw(std::bad_alloc());
      } else {
        return (NULL);
      }
    }

#ifdef UNIV_PFS_MEMORY
    ut_new_pfx_t *pfx = static_cast<ut_new_pfx_t *>(ptr);

    allocate_trace(total_bytes, key, pfx);

    return (reinterpret_cast<pointer>(pfx + 1));
#else
    return (reinterpret_cast<pointer>(ptr));
#endif /* UNIV_PFS_MEMORY */
  }

  /** Free a memory allocated by allocate() and trace the deallocation.
  @param[in,out]	ptr		pointer to memory to free
  @param[in]	n_elements	number of elements allocated (unused) */
  void deallocate(pointer ptr, size_type n_elements = 0) {
    if (ptr == NULL) {
      return;
    }

#ifdef UNIV_PFS_MEMORY
    ut_new_pfx_t *pfx = reinterpret_cast<ut_new_pfx_t *>(ptr) - 1;

    deallocate_trace(pfx);

    free(pfx);
#else
    free(ptr);
#endif /* UNIV_PFS_MEMORY */
  }

  /** Create an object of type 'T' using the value 'val' over the
  memory pointed by 'p'. */
  void construct(pointer p, const T &val) { new (p) T(val); }

  /** Destroy an object pointed by 'p'. */
  void destroy(pointer p) { p->~T(); }

  /** Return the address of an object. */
  pointer address(reference x) const { return (&x); }

  /** Return the address of a const object. */
  const_pointer address(const_reference x) const { return (&x); }

  template <class U>
  struct rebind {
    typedef ut_allocator<U> other;
  };

  /* The following are custom methods, not required by the standard. */

#ifdef UNIV_PFS_MEMORY

  /** realloc(3)-like method.
  The passed in ptr must have been returned by allocate() and the
  pointer returned by this method must be passed to deallocate() when
  no longer needed.
  @param[in,out]	ptr		old pointer to reallocate
  @param[in]	n_elements	new number of elements to allocate
  @param[in]	key		Performance schema key to allocate under
  @return newly allocated memory */
  pointer reallocate(void *ptr, size_type n_elements, PSI_memory_key key) {
    if (n_elements == 0) {
      deallocate(static_cast<pointer>(ptr));
      return (NULL);
    }

    if (ptr == NULL) {
      return (allocate(n_elements, NULL, key, false, false));
    }

    if (n_elements > max_size()) {
      return (NULL);
    }

    ut_new_pfx_t *pfx_old;
    ut_new_pfx_t *pfx_new;
    size_t total_bytes;

    pfx_old = reinterpret_cast<ut_new_pfx_t *>(ptr) - 1;

    total_bytes = n_elements * sizeof(T) + sizeof(ut_new_pfx_t);

    for (size_t retries = 1;; retries++) {
      pfx_new = static_cast<ut_new_pfx_t *>(realloc(pfx_old, total_bytes));

      if (pfx_new != NULL || retries >= alloc_max_retries) {
        break;
      }

      os_thread_sleep(1000000 /* 1 second */);
    }

    if (pfx_new == NULL) {
      ib::fatal_or_error(m_oom_fatal)
          << "Cannot reallocate " << total_bytes << " bytes of memory after "
          << alloc_max_retries << " retries over " << alloc_max_retries
          << " seconds. OS error: " << strerror(errno) << " (" << errno << "). "
          << OUT_OF_MEMORY_MSG;
      /* not reached */
      return (NULL);
    }

    /* pfx_new still contains the description of the old block
    that was presumably freed by realloc(). */
    deallocate_trace(pfx_new);

    /* pfx_new is set here to describe the new block. */
    allocate_trace(total_bytes, key, pfx_new);

    return (reinterpret_cast<pointer>(pfx_new + 1));
  }

  /** Allocate, trace the allocation and construct 'n_elements' objects
  of type 'T'. If the allocation fails or if some of the constructors
  throws an exception, then this method will return NULL. It does not
  throw exceptions. After successful completion the returned pointer
  must be passed to delete_array() when no longer needed.
  @param[in]	n_elements	number of elements to allocate
  @param[in]	key		Performance schema key to allocate under
  @return pointer to the first allocated object or NULL */
  pointer new_array(size_type n_elements, PSI_memory_key key) {
    static_assert(std::is_default_constructible<T>::value,
                  "Array element type must be default-constructible");

    T *p = allocate(n_elements, NULL, key, false, false);

    if (p == NULL) {
      return (NULL);
    }

    T *first = p;
    size_type i;

    try {
      for (i = 0; i < n_elements; i++) {
        new (p) T;
        ++p;
      }
    } catch (...) {
      for (size_type j = 0; j < i; j++) {
        --p;
        p->~T();
      }

      deallocate(first);

      throw;
    }

    return (first);
  }

  /** Destroy, deallocate and trace the deallocation of an array created
  by new_array().
  @param[in,out]	ptr	pointer to the first object in the array */
  void delete_array(T *ptr) {
    if (ptr == NULL) {
      return;
    }

    const size_type n_elements = n_elements_allocated(ptr);

    T *p = ptr + n_elements - 1;

    for (size_type i = 0; i < n_elements; i++) {
      p->~T();
      --p;
    }

    deallocate(ptr);
  }

#endif /* UNIV_PFS_MEMORY */

  /** Allocate a large chunk of memory that can hold 'n_elements'
  objects of type 'T' and trace the allocation.
  @param[in]	n_elements	number of elements
  @param[out]	pfx		storage for the description of the
  allocated memory. The caller must provide space for this one and keep
  it until the memory is no longer needed and then pass it to
  deallocate_large().
  @return pointer to the allocated memory or NULL */
  pointer allocate_large(size_type n_elements, ut_new_pfx_t *pfx) {
    if (n_elements == 0 || n_elements > max_size()) {
      return (NULL);
    }

    ulint n_bytes = n_elements * sizeof(T);

    pointer ptr = reinterpret_cast<pointer>(os_mem_alloc_large(&n_bytes));

#ifdef UNIV_PFS_MEMORY
    if (ptr != NULL) {
      allocate_trace(n_bytes, PSI_NOT_INSTRUMENTED, pfx);
    }
#else
    pfx->m_size = n_bytes;
#endif /* UNIV_PFS_MEMORY */

    return (ptr);
  }

  /** Free a memory allocated by allocate_large() and trace the
  deallocation.
  @param[in,out]	ptr	pointer to memory to free
  @param[in]	pfx	descriptor of the memory, as returned by
  allocate_large(). */
  void deallocate_large(pointer ptr, const ut_new_pfx_t *pfx) {
#ifdef UNIV_PFS_MEMORY
    deallocate_trace(pfx);
#endif /* UNIV_PFS_MEMORY */

    os_mem_free_large(ptr, pfx->m_size);
  }

 private:
#ifdef UNIV_PFS_MEMORY

  /** Retrieve the size of a memory block allocated by new_array().
  @param[in]	ptr	pointer returned by new_array().
  @return size of memory block */
  size_type n_elements_allocated(const_pointer ptr) {
    const ut_new_pfx_t *pfx = reinterpret_cast<const ut_new_pfx_t *>(ptr) - 1;

    const size_type user_bytes = pfx->m_size - sizeof(ut_new_pfx_t);

    ut_ad(user_bytes % sizeof(T) == 0);

    return (user_bytes / sizeof(T));
  }

  /** Trace a memory allocation.
  @param[in]	size	number of bytes that were allocated
  @param[in]	key	Performance Schema key
  @param[out]	pfx	placeholder to store the info which will be
                          needed when freeing the memory */
  void allocate_trace(size_t size, PSI_memory_key key, ut_new_pfx_t *pfx) {
    if (m_key != PSI_NOT_INSTRUMENTED) {
      key = m_key;
    }

    pfx->m_key = PSI_MEMORY_CALL(memory_alloc)(key, size, &pfx->m_owner);

    pfx->m_size = size;
  }

  /** Trace a memory deallocation.
  @param[in]	pfx	info for the deallocation */
  void deallocate_trace(const ut_new_pfx_t *pfx) {
    PSI_MEMORY_CALL(memory_free)(pfx->m_key, pfx->m_size, pfx->m_owner);
  }
#endif /* UNIV_PFS_MEMORY */

  /* Assignment operator, not used, thus disabled (private. */
  template <class U>
  void operator=(const ut_allocator<U> &);

#ifdef UNIV_PFS_MEMORY
  /** Performance schema key. */
  PSI_memory_key m_key;
#endif /* UNIV_PFS_MEMORY */

  /** A flag to indicate whether out of memory (OOM) error is considered
  fatal.  If true, it is fatal. */
  bool m_oom_fatal;
};

/** Compare two allocators of the same type.
As long as the type of A1 and A2 is the same, a memory allocated by A1
could be freed by A2 even if the pfs mem key is different. */
template <typename T>
inline bool operator==(const ut_allocator<T> &lhs, const ut_allocator<T> &rhs) {
  return (true);
}

/** Compare two allocators of the same type. */
template <typename T>
inline bool operator!=(const ut_allocator<T> &lhs, const ut_allocator<T> &rhs) {
  return (!(lhs == rhs));
}
