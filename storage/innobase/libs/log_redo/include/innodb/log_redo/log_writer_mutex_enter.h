#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

#define log_writer_mutex_enter(log) mutex_enter(&((log).writer_mutex))
