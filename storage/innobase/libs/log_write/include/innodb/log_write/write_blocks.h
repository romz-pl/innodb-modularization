#pragma once

#include <innodb/univ/univ.h>

struct log_t;

namespace Log_files_write_impl {

void write_blocks(log_t &log, byte *write_buf, size_t write_size,
                                uint64_t real_offset);

}
