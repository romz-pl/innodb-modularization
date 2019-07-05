#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_t.h>
#include <innodb/io/os_offset_t.h>

bool os_file_seek(const char *pathname, os_file_t file, os_offset_t offset);
