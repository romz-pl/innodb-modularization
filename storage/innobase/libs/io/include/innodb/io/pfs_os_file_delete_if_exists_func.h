#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/pfs/mysql_pfs_key_t.h>

bool pfs_os_file_delete_if_exists_func(mysql_pfs_key_t key, const char *name,
                                       bool *exist, const char *src_file,
                                       uint src_line);

#endif
