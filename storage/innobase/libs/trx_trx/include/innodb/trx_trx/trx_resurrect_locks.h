#pragma once

#include <innodb/univ/univ.h>

/** Resurrect table locks for resurrected transactions. */
void trx_resurrect_locks();
