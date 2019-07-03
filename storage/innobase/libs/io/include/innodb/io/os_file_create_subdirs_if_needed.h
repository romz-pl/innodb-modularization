#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

dberr_t os_file_create_subdirs_if_needed(const char *path);
