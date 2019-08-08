#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

struct log_t;

namespace Log_files_write_impl {

void validate_start_lsn(const log_t &log, lsn_t start_lsn,
                                      size_t buffer_size);

}
