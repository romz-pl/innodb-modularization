#pragma once

#include <innodb/univ/univ.h>

#include <cstdio>

struct trx_t;

/** Set detailed error message for the transaction from a file. Note that the
 file is rewinded before reading from it. */
void trx_set_detailed_error_from_file(
    trx_t *trx,  /*!< in: transaction struct */
    FILE *file); /*!< in: file to read message from */
