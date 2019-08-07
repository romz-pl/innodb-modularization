#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

#define log_closer_mutex_own(log) \
  (mutex_own(&((log).closer_mutex)) || !(log).closer_thread_alive.load())
