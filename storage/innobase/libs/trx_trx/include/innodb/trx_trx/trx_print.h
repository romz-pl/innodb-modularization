#pragma once

#include <innodb/univ/univ.h>

#include <cstdio>

struct trx_t;

/** Prints info about a transaction.
 Acquires and releases lock_sys->mutex and trx_sys->mutex. */
void trx_print(FILE *f,              /*!< in: output stream */
               const trx_t *trx,     /*!< in: transaction */
               ulint max_query_len); /*!< in: max query length to print,
                                     or 0 to use the default max length */
