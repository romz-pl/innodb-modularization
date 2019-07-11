#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/error/dberr_t.h>
#include <innodb/io/pfs_os_file_t.h>

class IORequest;

dberr_t pfs_os_file_read_first_page_func(IORequest &type, pfs_os_file_t file,
                                         void *buf, ulint n,
                                         const char *src_file, uint src_line);

#endif /* UNIV_PFS_IO */
