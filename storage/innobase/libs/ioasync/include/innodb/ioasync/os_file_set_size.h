#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/pfs_os_file_t.h>
#include <innodb/io/os_offset_t.h>

bool os_file_set_size(const char *name, pfs_os_file_t file, os_offset_t offset,
                      os_offset_t size, bool read_only, bool flush)
    MY_ATTRIBUTE((warn_unused_result));
