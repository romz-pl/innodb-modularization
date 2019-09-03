#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/sync_mutex/mutex_enter.h>

/** Acquire the trx->mutex. */
#define trx_mutex_enter(t)  \
  do {                      \
    mutex_enter(&t->mutex); \
  } while (0)
