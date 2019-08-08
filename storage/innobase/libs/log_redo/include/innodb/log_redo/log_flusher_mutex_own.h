#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>
#include <innodb/sync_mutex/mutex_own.h>

#define log_flusher_mutex_own(log) \
  (mutex_own(&((log).flusher_mutex)) || !(log).flusher_thread_alive.load())
