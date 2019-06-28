#pragma once

#include <innodb/univ/univ.h>

/** A structure that holds the necessary data for performance schema
accounting. An object of this type is put in front of each allocated block
of memory when allocation is done by ut_allocator::allocate(). This is
because the data is needed even when freeing the memory. Users of
ut_allocator::allocate_large() are responsible for maintaining this
themselves. */
struct ut_new_pfx_t {
#ifdef UNIV_PFS_MEMORY

  /** Performance schema key. Assigned to a name at startup via
  PSI_MEMORY_CALL(register_memory)() and later used for accounting
  allocations and deallocations with
  PSI_MEMORY_CALL(memory_alloc)(key, size, owner) and
  PSI_MEMORY_CALL(memory_free)(key, size, owner). */
  PSI_memory_key m_key;

  /**
    Thread owner.
    Instrumented thread that owns the allocated memory.
    This state is used by the performance schema to maintain
    per thread statistics,
    when memory is given from thread A to thread B.
  */
  struct PSI_thread *m_owner;

#endif /* UNIV_PFS_MEMORY */

  /** Size of the allocated block in bytes, including this prepended
  aux structure (for ut_allocator::allocate()). For example if InnoDB
  code requests to allocate 100 bytes, and sizeof(ut_new_pfx_t) is 16,
  then 116 bytes are allocated in total and m_size will be 116.
  ut_allocator::allocate_large() does not prepend this struct to the
  allocated block and its users are responsible for maintaining it
  and passing it later to ut_allocator::deallocate_large(). */
  size_t m_size;
#if SIZEOF_VOIDP == 4
  /** Pad the header size to a multiple of 64 bits on 32-bit systems,
  so that the payload will be aligned to 64 bits. */
  size_t pad;
#endif
};
