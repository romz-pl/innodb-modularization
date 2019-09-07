#pragma once

#include <innodb/univ/univ.h>

struct trx_undo_t;

/**
Frees an undo log memory copy. */
void trx_undo_mem_free(trx_undo_t *undo); /* in: the undo object to be freed */
