#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Calculates time that elapsed since last checkpoint.
@return number of microseconds since the last checkpoint */
uint64_t log_checkpoint_time_elapsed(const log_t &log);
