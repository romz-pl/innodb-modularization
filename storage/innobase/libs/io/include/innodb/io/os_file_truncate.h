#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/pfs_os_file_t.h>
#include <innodb/io/os_offset_t.h>

bool os_file_truncate(const char *pathname, pfs_os_file_t file, os_offset_t size);
