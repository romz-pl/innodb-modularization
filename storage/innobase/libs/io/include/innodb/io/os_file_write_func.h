#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_file_t.h>
#include <innodb/io/os_offset_t.h>

class IORequest;

dberr_t os_file_write_func(IORequest &type, const char *name, os_file_t file,
                           const void *buf, os_offset_t offset, ulint n) MY_ATTRIBUTE((warn_unused_result));
