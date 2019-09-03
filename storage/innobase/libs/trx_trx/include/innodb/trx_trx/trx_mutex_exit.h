#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/sync_mutex/mutex_exit.h>

/** Release the trx->mutex. */
#define trx_mutex_exit(t)  \
  do {                     \
    mutex_exit(&t->mutex); \
  } while (0)
