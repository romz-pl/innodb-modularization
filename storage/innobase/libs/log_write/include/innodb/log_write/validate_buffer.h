#pragma once

#include <innodb/univ/univ.h>

struct log_t;

namespace Log_files_write_impl {

void validate_buffer(const log_t &log, const byte *buffer,
                                   size_t buffer_size);

}
