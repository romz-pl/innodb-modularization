#pragma once

#include <innodb/univ/univ.h>

#include <atomic>

/** Function used to calculate checksums of log blocks. */
typedef std::atomic<uint32_t (*)(const byte *log_block)> log_checksum_func_t;
