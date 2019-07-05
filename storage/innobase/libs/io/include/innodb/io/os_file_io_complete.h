#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_t.h>
#include <innodb/io/os_offset_t.h>

dberr_t os_file_io_complete(const IORequest &type, os_file_t fh,
                                   byte *buf, byte *scratch, ulint src_len,
                                   os_offset_t offset, ulint len);
