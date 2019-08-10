#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/checkpoint_no_t.h>
#include <innodb/log_sn/lsn_t.h>

struct log_t;

namespace Log_files_write_impl {

void prepare_full_blocks(const log_t &log, byte *buffer,
                                       size_t size, lsn_t start_lsn,
                                       checkpoint_no_t checkpoint_no);

}
