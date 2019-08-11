#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** @return true iff log threads are started */
bool log_threads_active(const log_t &log);
