#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Starts the transaction if it is not yet started. */
void trx_start_if_not_started_low(
    trx_t *trx,       /*!< in/out: transaction */
    bool read_write); /*!< in: true if read write transaction */
