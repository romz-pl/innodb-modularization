#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_file_t.h>
#include <innodb/io/os_offset_t.h>

dberr_t os_file_punch_hole_posix(os_file_t fh, os_offset_t off, os_offset_t len);
