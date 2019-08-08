#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>
#include <innodb/sync_mutex/mutex_enter.h>

#define log_write_notifier_mutex_enter(log) \
  mutex_enter(&((log).write_notifier_mutex))
