#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Test if trx->mutex is owned. */
#define trx_mutex_own(t) mutex_own(&t->mutex)
