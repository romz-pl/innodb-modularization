#pragma once

#include <innodb/univ/univ.h>

struct trx_t;
struct trx_named_savept_t;

/** Frees savepoint structs starting from savep. */
void trx_roll_savepoints_free(
    trx_t *trx,                 /*!< in: transaction handle */
    trx_named_savept_t *savep); /*!< in: free all savepoints > this one;
                                if this is NULL, free all savepoints
                                of trx */
