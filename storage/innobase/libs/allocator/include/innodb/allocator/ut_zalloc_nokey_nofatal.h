#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_allocator.h>

#define ut_zalloc_nokey_nofatal(n_bytes)       \
  static_cast<void *>(                         \
      ut_allocator<byte>(PSI_NOT_INSTRUMENTED) \
          .set_oom_not_fatal()                 \
          .allocate(n_bytes, NULL, UT_NEW_THIS_FILE_PSI_KEY, true, false))

#else

#define ut_zalloc_nokey_nofatal(n_bytes) ::calloc(1, n_bytes)

#endif

