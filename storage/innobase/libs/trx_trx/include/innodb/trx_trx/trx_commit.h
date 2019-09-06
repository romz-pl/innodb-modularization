#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Commits a transaction. */
void trx_commit(trx_t *trx); /*!< in/out: transaction */
