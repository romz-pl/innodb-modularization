#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>
#include <innodb/sync_mutex/mutex_exit.h>

#define log_flush_notifier_mutex_exit(log) \
  mutex_exit(&((log).flush_notifier_mutex))
