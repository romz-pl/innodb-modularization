#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/error/dberr_t.h>
#include <innodb/io/pfs_os_file_t.h>
#include <innodb/io/os_offset_t.h>

class IORequest;

dberr_t pfs_os_file_read_func(IORequest &type, pfs_os_file_t file, void *buf,
                              os_offset_t offset, ulint n, const char *src_file,
                              uint src_line);

#endif

