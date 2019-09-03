#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

bool trx_is_rseg_assigned(const trx_t *trx);
