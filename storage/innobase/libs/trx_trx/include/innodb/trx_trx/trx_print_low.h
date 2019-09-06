#pragma once

#include <innodb/univ/univ.h>

#include <cstdio>

struct trx_t;

/** Prints info about a transaction.
 Caller must hold trx_sys->mutex. */
void trx_print_low(FILE *f,
                   /*!< in: output stream */
                   const trx_t *trx,
                   /*!< in: transaction */
                   ulint max_query_len,
                   /*!< in: max query length to print,
                   or 0 to use the default max length */
                   ulint n_rec_locks,
                   /*!< in: lock_number_of_rows_locked(&trx->lock) */
                   ulint n_trx_locks,
                   /*!< in: length of trx->lock.trx_locks */
                   ulint heap_size); /*!< in: mem_heap_get_size(trx->lock.lock_heap) */
