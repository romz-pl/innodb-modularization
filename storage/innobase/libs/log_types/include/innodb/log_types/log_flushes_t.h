#pragma once

#include <innodb/univ/univ.h>

#include <atomic>

/** Type used for counters in log_t: flushes_requested and flushes_expected.
They represent number of requests to flush the redo log to disk. */
typedef std::atomic<int64_t> log_flushes_t;
