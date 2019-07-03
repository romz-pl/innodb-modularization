#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_type_t.h>

bool os_file_status(const char *path, bool *exists, os_file_type_t *type);
