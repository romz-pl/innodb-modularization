#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_t.h>

os_file_t os_file_create_simple_func(const char *name, ulint create_mode,
                                     ulint access_type, bool read_only,
                                     bool *success);
