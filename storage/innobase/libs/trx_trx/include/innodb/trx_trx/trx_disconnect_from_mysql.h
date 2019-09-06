#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

void trx_disconnect_from_mysql(trx_t *trx, bool prepared);
