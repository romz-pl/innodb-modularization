#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_types/lock_mode.h>

struct lock_t;
struct trx_t;
struct dict_table_t;

/** Checks if a transaction has the specified table lock, or stronger. This
function should only be called by the thread that owns the transaction.
@param[in]	trx	transaction
@param[in]	table	table
@param[in]	mode	lock mode
@return lock or NULL */
UNIV_INLINE
const lock_t *lock_table_has(const trx_t *trx, const dict_table_t *table,
                             enum lock_mode mode);
