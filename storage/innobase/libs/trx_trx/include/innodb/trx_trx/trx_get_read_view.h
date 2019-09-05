#pragma once

#include <innodb/univ/univ.h>

struct trx_t;
class ReadView;

ReadView *trx_get_read_view(trx_t *trx);

const ReadView *trx_get_read_view(const trx_t *trx);
