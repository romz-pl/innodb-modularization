#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/tablespace/AIO_mode.h>
#include <innodb/io/pfs_os_file_t.h>
#include <innodb/io/os_offset_t.h>

class IORequest;
struct fil_node_t;

dberr_t os_aio_func(IORequest &type, AIO_mode aio_mode, const char *name,
                    pfs_os_file_t file, void *buf, os_offset_t offset, ulint n,
                    bool read_only, fil_node_t *m1, void *m2);
