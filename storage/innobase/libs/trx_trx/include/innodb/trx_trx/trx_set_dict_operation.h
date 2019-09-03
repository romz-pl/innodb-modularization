#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_dict_op_t.h>

struct trx_t;

/** Flag a transaction a dictionary operation.
@param[in,out]	trx	transaction
@param[in]	op	operation, not TRX_DICT_OP_NONE */
void trx_set_dict_operation(trx_t *trx, enum trx_dict_op_t op);
