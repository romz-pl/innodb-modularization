#include <innodb/allocator/os_mem_free_large.h>

#include <innodb/allocator/os_large_page_size.h>
#include <innodb/allocator/os_total_large_mem_allocated.h>
#include <innodb/allocator/os_use_large_pages.h>
#include <innodb/assert/assert.h>
#include <innodb/atomic/atomic.h>
#include <innodb/logger/error.h>
#include <innodb/memory_check/memory_check.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "MAP_ANON.h"

void ut_free(void*);

/** Frees large pages memory.
@param[in]	ptr	pointer returned by os_mem_alloc_large()
@param[in]	size	size returned by os_mem_alloc_large() */
void os_mem_free_large(void *ptr, ulint size) {
  ut_a(os_total_large_mem_allocated >= size);

#if defined HAVE_LINUX_LARGE_PAGES && defined UNIV_LINUX
  if (os_use_large_pages && os_large_page_size && !shmdt(ptr)) {
    os_atomic_decrement_ulint(&os_total_large_mem_allocated, size);
    UNIV_MEM_FREE(ptr, size);
    return;
  }
#endif /* HAVE_LINUX_LARGE_PAGES && UNIV_LINUX */
#ifdef _WIN32
  /* When RELEASE memory, the size parameter must be 0.
  Do not use MEM_RELEASE with MEM_DECOMMIT. */
  if (!VirtualFree(ptr, 0, MEM_RELEASE)) {
    ib::error(ER_IB_MSG_857) << "VirtualFree(" << ptr << ", " << size
                             << ") failed; Windows error " << GetLastError();
  } else {
    os_atomic_decrement_ulint(&os_total_large_mem_allocated, size);
    UNIV_MEM_FREE(ptr, size);
  }
#elif !defined OS_MAP_ANON
  ut_free(ptr);
#else
#if defined(UNIV_SOLARIS)
  if (munmap(static_cast<caddr_t>(ptr), size)) {
#else
  if (munmap(ptr, size)) {
#endif /* UNIV_SOLARIS */
    ib::error(ER_IB_MSG_858) << "munmap(" << ptr << ", " << size
                             << ") failed;"
                                " errno "
                             << errno;
  } else {
    os_atomic_decrement_ulint(&os_total_large_mem_allocated, size);
    UNIV_MEM_FREE(ptr, size);
  }
#endif
}
