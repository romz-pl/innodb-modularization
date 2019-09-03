#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/**
Release the transaction. Decrease the reference count.
@param trx Transaction that is being released */
void trx_release_reference(trx_t *trx);
