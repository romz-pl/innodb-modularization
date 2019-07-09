#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/error/dberr_t.h>
#include <innodb/io/pfs_os_file_t.h>
#include <innodb/io/os_offset_t.h>

dberr_t pfs_os_file_copy_func(pfs_os_file_t src, os_offset_t src_offset,
                              pfs_os_file_t dest, os_offset_t dest_offset,
                              uint size, const char *src_file, uint src_line);


#endif
