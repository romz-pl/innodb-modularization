#pragma once

#include <innodb/univ/univ.h>

int add_redo_file_callback(char *file_name, ib_uint64_t file_size,
                                  ib_uint64_t file_offset, void *context);
