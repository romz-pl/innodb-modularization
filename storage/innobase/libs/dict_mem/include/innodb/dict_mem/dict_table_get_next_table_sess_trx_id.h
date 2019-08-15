#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Get table session trx-id and increment the trx-id counter for next use.
@param[in,out]	table	table handler
@return next table session trx-id. */
UNIV_INLINE
trx_id_t dict_table_get_next_table_sess_trx_id(dict_table_t *table) {
  return (++table->sess_trx_id);
}
