#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>

/**
Check if the transaction is being referenced. */
#define trx_is_referenced(t) ((t)->n_ref > 0)
