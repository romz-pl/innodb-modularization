#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Compares the "weight" (or size) of two transactions. Transactions that
 have edited non-transactional tables are considered heavier than ones
 that have not.
 @return true if weight(a) >= weight(b) */
bool trx_weight_ge(const trx_t *a,  /*!< in: the transaction to be compared */
                   const trx_t *b); /*!< in: the transaction to be compared */
