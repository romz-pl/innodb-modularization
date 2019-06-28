#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_allocator.h>

#define ut_realloc(ptr, n_bytes)                               \
  static_cast<void *>(ut_allocator<byte>(PSI_NOT_INSTRUMENTED) \
                          .reallocate(ptr, n_bytes, UT_NEW_THIS_FILE_PSI_KEY))

#else

#define ut_realloc(ptr, n_bytes) ::realloc(ptr, n_bytes)

#endif
