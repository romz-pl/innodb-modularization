#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/PoolManager.h>
#include <innodb/trx_trx/trx_pool_t.h>
#include <innodb/trx_trx/TrxPoolManagerLock.h>

typedef PoolManager<trx_pool_t, TrxPoolManagerLock> trx_pools_t;
