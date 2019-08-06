#pragma once

#include <innodb/univ/univ.h>

/** The state of a log group */
enum class log_state_t {
  /** No corruption detected */
  OK,
  /** Corrupted */
  CORRUPTED
};
