#pragma once

#include <innodb/univ/univ.h>

struct trx_t;
struct trx_undo_ptr_t;
struct trx_undo_t;

void trx_resurrect_table_ids(trx_t *trx, const trx_undo_ptr_t *undo_ptr,
                                    const trx_undo_t *undo);
