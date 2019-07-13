#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_offset_t.h>

struct fil_node_t;

dberr_t fil_write_zeros(const fil_node_t *file, ulint page_size,
                               os_offset_t start, ulint len,
                               bool read_only_mode);
