#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_t.h>
#include <innodb/io/os_offset_t.h>
#include <innodb/error/dberr_t.h>

class IORequest;

MY_ATTRIBUTE((warn_unused_result)) ssize_t
    os_file_pwrite(IORequest &type, os_file_t file, const byte *buf, ulint n,
                   os_offset_t offset, dberr_t *err);
