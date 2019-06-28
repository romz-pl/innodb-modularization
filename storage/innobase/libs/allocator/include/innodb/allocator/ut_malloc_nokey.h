#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_allocator.h>

#define ut_malloc_nokey(n_bytes)               \
  static_cast<void *>(                         \
      ut_allocator<byte>(PSI_NOT_INSTRUMENTED) \
          .allocate(n_bytes, NULL, UT_NEW_THIS_FILE_PSI_KEY, false, false))

#else

#define ut_malloc_nokey(n_bytes) ::malloc(n_bytes)

#endif
