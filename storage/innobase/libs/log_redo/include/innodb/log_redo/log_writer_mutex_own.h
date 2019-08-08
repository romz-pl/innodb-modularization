#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>
#include <innodb/sync_mutex/mutex_own.h>

#define log_writer_mutex_own(log) \
  (mutex_own(&((log).writer_mutex)) || !(log).writer_thread_alive.load())
