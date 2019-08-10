#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

#define log_checkpointer_mutex_own(log)      \
  (mutex_own(&((log).checkpointer_mutex)) || \
   !(log).checkpointer_thread_alive.load())
