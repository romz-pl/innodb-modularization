#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/pfs_os_file_close_func.h>

#define os_file_close_pfs(file) pfs_os_file_close_func(file, __FILE__, __LINE__)
