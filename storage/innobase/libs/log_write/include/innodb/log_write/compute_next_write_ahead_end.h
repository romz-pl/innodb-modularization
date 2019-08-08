#pragma once

#include <innodb/univ/univ.h>

struct log_t;

namespace Log_files_write_impl {

uint64_t compute_next_write_ahead_end(uint64_t real_offset);

}
