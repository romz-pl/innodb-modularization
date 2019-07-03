#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_size_t.h>

os_file_size_t os_file_get_size(const char *filename) MY_ATTRIBUTE((warn_unused_result));
