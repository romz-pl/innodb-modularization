#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/MutexMonitor.h>

/** For monitoring active mutexes */
extern MutexMonitor *mutex_monitor;
