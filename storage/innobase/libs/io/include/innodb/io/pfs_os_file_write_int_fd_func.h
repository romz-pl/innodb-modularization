#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_offset_t.h>

class IORequest;

dberr_t pfs_os_file_write_int_fd_func(IORequest &type, const char *name,
                                      int file, const void *buf,
                                      os_offset_t offset, ulint n,
                                      const char *src_file, ulint src_line);


#endif
