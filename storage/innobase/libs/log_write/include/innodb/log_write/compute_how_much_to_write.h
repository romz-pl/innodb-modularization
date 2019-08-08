#pragma once

#include <innodb/univ/univ.h>

struct log_t;

namespace Log_files_write_impl {

size_t compute_how_much_to_write(const log_t &log,
                                               uint64_t real_offset,
                                               size_t buffer_size,
                                               bool &write_from_log_buffer);

}
