#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/sn_t.h>

#include <atomic>

/** Alias for atomic based on sn_t. */
using atomic_sn_t = std::atomic<sn_t>;
