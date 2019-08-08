#pragma once

#include <innodb/univ/univ.h>

namespace Log_files_write_impl {

bool write_ahead_enough(uint64_t write_ahead_end, uint64_t offset,
                                      size_t size);

}
