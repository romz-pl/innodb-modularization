#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include "pfs_file_provider.h"

/* Following four macros are instumentations to register
various file I/O operations with performance schema.
1) register_pfs_file_open_begin() and register_pfs_file_open_end() are
used to register file creation, opening and closing.
2) register_pfs_file_rename_begin() and  register_pfs_file_rename_end()
are used to register file renaming.
3) register_pfs_file_io_begin() and register_pfs_file_io_end() are
used to register actual file read, write and flush
4) register_pfs_file_close_begin() and register_pfs_file_close_end()
are used to register file deletion operations*/

#define register_pfs_file_io_end(locker, count)    \
  do {                                             \
    if (locker != NULL) {                          \
      PSI_FILE_CALL(end_file_wait)(locker, count); \
    }                                              \
  } while (0)


#endif
