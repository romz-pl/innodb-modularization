#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_file_stat_t.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

dberr_t os_file_get_status_posix(const char *path,
                                 os_file_stat_t *stat_info,
                                 struct stat *statinfo,
                                 bool check_rw_perm,
                                 bool read_only);
