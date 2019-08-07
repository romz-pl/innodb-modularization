#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

#define log_closer_mutex_exit(log) mutex_exit(&((log).closer_mutex))
