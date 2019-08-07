#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>
#include <innodb/sync_mutex/mutex_enter.h>

#define log_closer_mutex_enter(log) mutex_enter(&((log).closer_mutex))
