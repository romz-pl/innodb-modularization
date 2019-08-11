#pragma once

#include <innodb/univ/univ.h>

struct log_t;

bool log_calc_concurrency_margin(log_t &log);
