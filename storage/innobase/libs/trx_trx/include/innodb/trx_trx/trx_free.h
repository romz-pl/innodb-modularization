#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

void trx_free(trx_t *&trx);
