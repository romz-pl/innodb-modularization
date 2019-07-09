#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

bool pfs_os_file_rename_func(mysql_pfs_key_t key, const char *oldpath,
                             const char *newpath, const char *src_file,
                             uint src_line);


#endif
