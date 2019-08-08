#pragma once

#include <innodb/univ/univ.h>

struct log_t;

namespace Log_files_write_impl {

bool current_file_has_space(const log_t &log, uint64_t offset,
                                          size_t size);

}
