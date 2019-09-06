#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/**
Add the transaction to the RW transaction set
@param trx		transaction instance to add */
void trx_sys_rw_trx_add(trx_t *trx);
