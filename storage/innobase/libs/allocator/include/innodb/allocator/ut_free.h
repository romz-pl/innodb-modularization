#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_allocator.h>

#define ut_free(ptr)                       \
  ut_allocator<byte>(PSI_NOT_INSTRUMENTED) \
      .deallocate(reinterpret_cast<byte *>(ptr))

#else

#define ut_free(ptr) ::free(ptr)

#endif
