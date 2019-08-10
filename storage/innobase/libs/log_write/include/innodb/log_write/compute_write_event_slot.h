#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

struct log_t;

namespace Log_files_write_impl {

size_t compute_write_event_slot(const log_t &log, lsn_t lsn);

}
