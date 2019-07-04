#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_t.h>

int os_file_fsync_posix(os_file_t file);
