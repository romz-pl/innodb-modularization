#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/lst/lst.h>

/** Calculates the "weight" of a transaction. The weight of one transaction
 is estimated as the number of altered rows + the number of locked rows.
 @param t transaction
 @return transaction weight */
#define TRX_WEIGHT(t) ((t)->undo_no + UT_LIST_GET_LEN((t)->lock.trx_locks))
