#pragma once

#include <innodb/univ/univ.h>

struct log_t;

namespace Log_files_write_impl {

bool current_write_ahead_enough(const log_t &log, uint64_t offset,
                                              size_t size);

}
