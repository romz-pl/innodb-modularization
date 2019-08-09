#pragma once

#include <innodb/univ/univ.h>

struct log_t;

void log_flush_update_stats(log_t &log);
