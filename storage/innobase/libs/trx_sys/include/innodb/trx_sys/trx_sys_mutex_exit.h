#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_sys/trx_sys.h>

/** Release the trx_sys->mutex. */
#define trx_sys_mutex_exit() \
  do {                       \
    trx_sys->mutex.exit();   \
  } while (0)
