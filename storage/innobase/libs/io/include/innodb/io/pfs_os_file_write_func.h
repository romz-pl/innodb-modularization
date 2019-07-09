#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_offset_t.h>
#include <innodb/io/pfs_os_file_t.h>

class IORequest;

dberr_t pfs_os_file_write_func(IORequest &type, const char *name,
                               pfs_os_file_t file, const void *buf,
                               os_offset_t offset, ulint n,
                               const char *src_file, uint src_line);


#endif
