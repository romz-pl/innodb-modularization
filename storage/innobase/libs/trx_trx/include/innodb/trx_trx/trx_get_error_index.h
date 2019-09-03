#pragma once

#include <innodb/univ/univ.h>

struct trx_t;
struct dict_index_t;

/** Retrieves the error_info field from a trx.
 @return the error index */
const dict_index_t *trx_get_error_index(const trx_t *trx); /*!< in: trx object */
