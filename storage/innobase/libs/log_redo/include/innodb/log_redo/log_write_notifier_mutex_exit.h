#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

#define log_write_notifier_mutex_exit(log) \
  mutex_exit(&((log).write_notifier_mutex))
