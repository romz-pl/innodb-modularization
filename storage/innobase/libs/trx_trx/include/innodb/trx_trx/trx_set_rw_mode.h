#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/**
Set the transaction as a read-write transaction if it is not already
tagged as such.
@param[in,out] trx	Transaction that needs to be "upgraded" to RW from RO */
void trx_set_rw_mode(trx_t *trx);
