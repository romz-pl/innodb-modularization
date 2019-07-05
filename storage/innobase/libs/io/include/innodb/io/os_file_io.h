#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_t.h>
#include <innodb/io/os_offset_t.h>

MY_ATTRIBUTE((warn_unused_result)) ssize_t
    os_file_io(const IORequest &in_type, os_file_t file, void *buf, ulint n,
               os_offset_t offset, dberr_t *err);
