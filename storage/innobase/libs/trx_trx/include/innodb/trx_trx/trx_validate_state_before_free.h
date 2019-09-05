#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

void trx_validate_state_before_free(trx_t *trx);
