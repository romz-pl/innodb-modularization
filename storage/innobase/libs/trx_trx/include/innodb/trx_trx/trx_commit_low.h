#pragma once

#include <innodb/univ/univ.h>

struct trx_t;
struct mtr_t;

/** Commits a transaction and a mini-transaction. */
void trx_commit_low(
    trx_t *trx,  /*!< in/out: transaction */
    mtr_t *mtr); /*!< in/out: mini-transaction (will be committed),
                 or NULL if trx made no modifications */
