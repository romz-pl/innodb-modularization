#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

struct log_t;

namespace Log_files_write_impl {

void start_next_file(log_t &log, lsn_t start_lsn);

}
