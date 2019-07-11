#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/pfs_os_file_t.h>

pfs_os_file_t os_file_create_func(const char *name, ulint create_mode,
                                  ulint purpose, ulint type, bool read_only,
                                  bool *success) MY_ATTRIBUTE((warn_unused_result));
