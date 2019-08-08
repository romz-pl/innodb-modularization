#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>
#include <innodb/log_types/checkpoint_no_t.h>

struct log_t;

namespace Log_files_write_impl {

void copy_to_write_ahead_buffer(log_t &log, const byte *buffer,
                                              size_t &size, lsn_t start_lsn,
                                              checkpoint_no_t checkpoint_no);

}
