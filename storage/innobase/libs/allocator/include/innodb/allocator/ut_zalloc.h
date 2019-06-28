#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_allocator.h>

#define ut_zalloc(n_bytes, key)                         \
  static_cast<void *>(ut_allocator<byte>(key).allocate( \
      n_bytes, NULL, UT_NEW_THIS_FILE_PSI_KEY, true, false))

#else

#define ut_zalloc(n_bytes, key) ::calloc(1, n_bytes)

#endif
