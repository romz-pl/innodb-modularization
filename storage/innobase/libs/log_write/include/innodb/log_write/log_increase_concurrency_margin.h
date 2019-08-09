#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Increase concurrency_margin used inside log_free_check() calls. */
void log_increase_concurrency_margin(log_t &log);
