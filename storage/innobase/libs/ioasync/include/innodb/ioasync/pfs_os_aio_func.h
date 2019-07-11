#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO
#ifndef UNIV_HOTBACKUP

#include <innodb/error/dberr_t.h>
#include <innodb/tablespace/AIO_mode.h>
#include <innodb/io/pfs_os_file_t.h>
#include <innodb/io/os_offset_t.h>

class IORequest;
struct fil_node_t;

dberr_t pfs_os_aio_func(IORequest &type, AIO_mode aio_mode, const char *name,
                        pfs_os_file_t file, void *buf, os_offset_t offset,
                        ulint n, bool read_only, fil_node_t *m1, void *m2,
                        const char *src_file, uint src_line);

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_PFS_IO */
