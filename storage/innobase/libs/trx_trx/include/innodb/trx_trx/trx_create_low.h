#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

trx_t *trx_create_low();
