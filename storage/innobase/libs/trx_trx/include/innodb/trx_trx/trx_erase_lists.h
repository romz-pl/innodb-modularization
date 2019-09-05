#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

void trx_erase_lists(trx_t *trx, bool serialised);
