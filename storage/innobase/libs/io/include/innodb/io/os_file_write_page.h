#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_t.h>
#include <innodb/io/os_offset_t.h>
#include <innodb/error/dberr_t.h>

class IORequest;

MY_ATTRIBUTE((warn_unused_result)) dberr_t
    os_file_write_page(IORequest &type, const char *name, os_file_t file,
                       const byte *buf, os_offset_t offset, ulint n);
