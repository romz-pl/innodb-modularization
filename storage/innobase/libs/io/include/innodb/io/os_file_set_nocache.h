#pragma once

#include <innodb/univ/univ.h>

void os_file_set_nocache(int fd, const char *file_name, const char *operation_name);
