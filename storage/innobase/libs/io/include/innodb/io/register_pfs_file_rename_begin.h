#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/register_pfs_file_open_begin.h>

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

#define register_pfs_file_rename_begin(state, locker, key, op, name, src_file, \
                                       src_line)                               \
  register_pfs_file_open_begin(state, locker, key, op, name, src_file,         \
                               static_cast<uint>(src_line))


#endif
