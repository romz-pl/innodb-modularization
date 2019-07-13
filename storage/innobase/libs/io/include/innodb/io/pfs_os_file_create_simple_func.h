#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/pfs_os_file_t.h>
#include <innodb/pfs/mysql_pfs_key_t.h>

pfs_os_file_t pfs_os_file_create_simple_func(
    mysql_pfs_key_t key, const char *name, ulint create_mode, ulint access_type,
    bool read_only, bool *success, const char *src_file, uint src_line);

#endif
