#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** At shutdown, frees the undo logs of a PREPARED transaction. */
void trx_undo_free_prepared(trx_t *trx) /*!< in/out: PREPARED transaction */
    UNIV_COLD;
