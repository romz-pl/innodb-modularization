#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Retrieves transaction's que state in a human readable string. The string
 should not be free()'d or modified.
 @return string in the data segment */
const char *trx_get_que_state_str(const trx_t *trx); /*!< in: transaction */
