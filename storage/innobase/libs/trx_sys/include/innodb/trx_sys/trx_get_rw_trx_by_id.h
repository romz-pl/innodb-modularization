#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Looks for the trx instance with the given id in the rw trx_list.
 @return	the trx handle or NULL if not found */
trx_t *trx_get_rw_trx_by_id(trx_id_t trx_id); /*!< in: trx id to search for */
