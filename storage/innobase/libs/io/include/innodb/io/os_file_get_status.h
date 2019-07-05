#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_file_stat_t.h>

dberr_t os_file_get_status(const char *path, os_file_stat_t *stat_info,
                           bool check_rw_perm, bool read_only);
