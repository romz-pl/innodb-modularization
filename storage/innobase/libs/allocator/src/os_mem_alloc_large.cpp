#include <innodb/allocator/os_mem_alloc_large.h>

#include <innodb/allocator/os_large_page_size.h>
#include <innodb/allocator/os_total_large_mem_allocated.h>
#include <innodb/allocator/os_use_large_pages.h>
#include <innodb/atomic/atomic.h>
#include <innodb/logger/error.h>
#include <innodb/logger/warn.h>
#include <innodb/math/ut_2pow_round.h>
#include <innodb/memory_check/memory_check.h>

#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include "MAP_ANON.h"


/** Allocates large pages memory.
@param[in,out]	n	Number of bytes to allocate
@return allocated memory */
void *os_mem_alloc_large(ulint *n) {
  void *ptr;
  ulint size;
#if defined HAVE_LINUX_LARGE_PAGES && defined UNIV_LINUX
  int shmid;
  struct shmid_ds buf;

  if (!os_use_large_pages || !os_large_page_size) {
    goto skip;
  }

  /* Align block size to os_large_page_size */
  ut_ad(ut_is_2pow(os_large_page_size));
  size = ut_2pow_round(*n + (os_large_page_size - 1), os_large_page_size);

  shmid = shmget(IPC_PRIVATE, (size_t)size, SHM_HUGETLB | SHM_R | SHM_W);
  if (shmid < 0) {
    ib::warn(ER_IB_MSG_852)
        << "Failed to allocate " << size << " bytes. errno " << errno;
    ptr = NULL;
  } else {
    ptr = shmat(shmid, NULL, 0);
    if (ptr == (void *)-1) {
      ib::warn(ER_IB_MSG_853) << "Failed to attach shared memory segment,"
                                 " errno "
                              << errno;
      ptr = NULL;
    }

    /* Remove the shared memory segment so that it will be
    automatically freed after memory is detached or
    process exits */
    shmctl(shmid, IPC_RMID, &buf);
  }

  if (ptr) {
    *n = size;
    os_atomic_increment_ulint(&os_total_large_mem_allocated, size);

    UNIV_MEM_ALLOC(ptr, size);
    return (ptr);
  }

  ib::warn(ER_IB_MSG_854) << "Using conventional memory pool";
skip:
#endif /* HAVE_LINUX_LARGE_PAGES && UNIV_LINUX */

#ifdef _WIN32
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);

  /* Align block size to system page size */
  ut_ad(ut_is_2pow(system_info.dwPageSize));
  /* system_info.dwPageSize is only 32-bit. Casting to ulint is required
  on 64-bit Windows. */
  size = *n = ut_2pow_round(*n + (system_info.dwPageSize - 1),
                            (ulint)system_info.dwPageSize);
  ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!ptr) {
    ib::info(ER_IB_MSG_855) << "VirtualAlloc(" << size
                            << " bytes) failed;"
                               " Windows error "
                            << GetLastError();
  } else {
    os_atomic_increment_ulint(&os_total_large_mem_allocated, size);
    UNIV_MEM_ALLOC(ptr, size);
  }
#else
  size = getpagesize();
  /* Align block size to system page size */
  ut_ad(ut_is_2pow(size));
  size = *n = ut_2pow_round(*n + (size - 1), size);
  ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | OS_MAP_ANON, -1,
             0);
  if (UNIV_UNLIKELY(ptr == (void *)-1)) {
    ib::error(ER_IB_MSG_856) << "mmap(" << size
                             << " bytes) failed;"
                                " errno "
                             << errno;
    ptr = NULL;
  } else {
    os_atomic_increment_ulint(&os_total_large_mem_allocated, size);
    UNIV_MEM_ALLOC(ptr, size);
  }
#endif
  return (ptr);
}
