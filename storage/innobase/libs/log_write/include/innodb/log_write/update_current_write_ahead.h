#pragma once

#include <innodb/univ/univ.h>

struct log_t;

namespace Log_files_write_impl {

void update_current_write_ahead(log_t &log, uint64_t real_offset,
                                              size_t write_size);

}
