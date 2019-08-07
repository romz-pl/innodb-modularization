#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

#define log_checkpointer_mutex_exit(log) mutex_exit(&((log).checkpointer_mutex))
