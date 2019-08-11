#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>
#include <innodb/sync_mutex/mutex_own.h>

#define log_write_notifier_mutex_own(log)      \
  (mutex_own(&((log).write_notifier_mutex)) || \
   !(log).write_notifier_thread_alive.load())
