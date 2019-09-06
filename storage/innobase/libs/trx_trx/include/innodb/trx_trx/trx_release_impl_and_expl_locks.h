#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

void trx_release_impl_and_expl_locks(trx_t *trx, bool serialized);
