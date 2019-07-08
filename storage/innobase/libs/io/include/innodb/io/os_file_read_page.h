#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_file_t.h>
#include <innodb/io/os_offset_t.h>

class IORequest;

MY_ATTRIBUTE((warn_unused_result)) dberr_t
    os_file_read_page(IORequest &type, os_file_t file, void *buf,
                      os_offset_t offset, ulint n, ulint *o, bool exit_on_err);
