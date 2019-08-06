#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

#include <atomic>

/** Alias for atomic based on lsn_t. */
using atomic_lsn_t = std::atomic<lsn_t>;
