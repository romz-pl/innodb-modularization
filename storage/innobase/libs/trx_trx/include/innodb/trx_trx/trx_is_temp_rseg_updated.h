#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

bool trx_is_temp_rseg_updated(const trx_t *trx);
