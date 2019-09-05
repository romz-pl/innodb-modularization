#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/TrxFactory.h>
#include <innodb/trx_trx/TrxPoolLock.h>
#include <innodb/trx_trx/Pool.h>

/** Use explicit mutexes for the trx_t pool and its manager. */
typedef Pool<trx_t, TrxFactory, TrxPoolLock> trx_pool_t;
