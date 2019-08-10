#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_checksum_func_t.h>

/** Pointer to the log checksum calculation function. Changes are protected
by log_mutex_enter_all, which also stops the log background threads. */
extern log_checksum_func_t log_checksum_algorithm_ptr;
