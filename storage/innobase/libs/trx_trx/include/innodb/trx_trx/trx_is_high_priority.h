#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/**
@param[in] trx		Transaction to check
@return true if the transaction is a high priority transaction.*/
bool trx_is_high_priority(const trx_t *trx);
