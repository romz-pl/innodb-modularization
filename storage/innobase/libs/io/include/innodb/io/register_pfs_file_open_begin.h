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

#define register_pfs_file_open_begin(state, locker, key, op, name, src_file, \
                                     src_line)                               \
  do {                                                                       \
    locker = PSI_FILE_CALL(get_thread_file_name_locker)(state, key.m_value,  \
                                                        op, name, &locker);  \
    if (locker != NULL) {                                                    \
      PSI_FILE_CALL(start_file_open_wait)                                    \
      (locker, src_file, static_cast<uint>(src_line));                       \
    }                                                                        \
  } while (0)

#endif
