#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Set detailed error message for the transaction. */
void trx_set_detailed_error(trx_t *trx,       /*!< in: transaction struct */
                            const char *msg); /*!< in: detailed error message */
