#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>
#include <innodb/sync_mutex/mutex_enter_nowait.h>

#define log_writer_mutex_enter_nowait(log) \
  mutex_enter_nowait(&((log).writer_mutex))
