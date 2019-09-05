#pragma once

#include <innodb/univ/univ.h>

struct trx_t;
class FlushObserver;

/** Set flush observer for the transaction
@param[in,out]	trx		transaction struct
@param[in]	observer	flush observer */
void trx_set_flush_observer(trx_t *trx, FlushObserver *observer);
