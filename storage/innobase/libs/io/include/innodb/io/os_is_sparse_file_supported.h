#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/pfs_os_file_t.h>

bool os_is_sparse_file_supported(const char *path, pfs_os_file_t fh)
    MY_ATTRIBUTE((warn_unused_result));
