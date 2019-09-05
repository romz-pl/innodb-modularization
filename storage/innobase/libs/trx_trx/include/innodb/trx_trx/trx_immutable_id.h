#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Provides an id of the transaction which does not change over time.
Contrast this with trx->id and trx_get_id_for_print(trx) which change value once
a transaction can no longer be treated as read-only and becomes read-write.
@param[in]  trx   The transaction for which you want an immutable id
@return the transaction's immutable id */
uint64_t trx_immutable_id(const trx_t *trx);
