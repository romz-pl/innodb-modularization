#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>

/* Transaction isolation levels (trx->isolation_level) */
#define TRX_ISO_READ_UNCOMMITTED trx_t::READ_UNCOMMITTED
#define TRX_ISO_READ_COMMITTED trx_t::READ_COMMITTED
#define TRX_ISO_REPEATABLE_READ trx_t::REPEATABLE_READ
#define TRX_ISO_SERIALIZABLE trx_t::SERIALIZABLE
