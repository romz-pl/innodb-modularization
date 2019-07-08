#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/pfs_os_file_t.h>

bool pfs_os_file_close_func(pfs_os_file_t file, const char *src_file,
                            uint src_line);

#endif
