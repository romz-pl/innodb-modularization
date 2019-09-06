#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Prepares a transaction for commit/rollback. */
void trx_commit_or_rollback_prepare(trx_t *trx); /*!< in/out: transaction */
