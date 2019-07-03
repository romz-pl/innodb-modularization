#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/pfs_os_file_t.h>

pfs_os_file_t os_file_create_simple_no_error_handling_func(const char *name,
                                                           ulint create_mode,
                                                           ulint access_type,
                                                           bool read_only,
                                                           bool *success);
