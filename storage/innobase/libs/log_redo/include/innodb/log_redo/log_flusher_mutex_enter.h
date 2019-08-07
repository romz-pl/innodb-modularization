#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

#define log_flusher_mutex_enter(log) mutex_enter(&((log).flusher_mutex))
