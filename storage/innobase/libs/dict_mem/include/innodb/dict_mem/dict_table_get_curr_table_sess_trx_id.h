#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Get current session trx-id.
@param[in]	table	table handler
@return table session trx-id. */
UNIV_INLINE
trx_id_t dict_table_get_curr_table_sess_trx_id(const dict_table_t *table) {
  return (table->sess_trx_id);
}
