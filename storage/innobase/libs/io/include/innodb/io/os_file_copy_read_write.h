#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_file_t.h>
#include <innodb//io/os_offset_t.h>

dberr_t os_file_copy_read_write(os_file_t src_file,
                                       os_offset_t src_offset,
                                       os_file_t dest_file,
                                       os_offset_t dest_offset, uint size);
