#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_sys/trx_sys.h>
#include <innodb/sync_mutex/mutex_enter.h>

/** Acquire the trx_sys->mutex. */
#define trx_sys_mutex_enter()     \
  do {                            \
    mutex_enter(&trx_sys->mutex); \
  } while (0)
