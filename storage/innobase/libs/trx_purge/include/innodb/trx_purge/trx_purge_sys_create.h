#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/purge_pq_t.h>

/** Creates the global purge system control structure and inits the history
mutex.
@param[in]      n_purge_threads   number of purge threads
@param[in,out]  purge_queue       UNDO log min binary heap */
void trx_purge_sys_create(ulint n_purge_threads, purge_pq_t *purge_queue);
