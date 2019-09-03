#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_dict_op_t.h>

struct trx_t;

/** Determine if a transaction is a dictionary operation.
 @return dictionary operation mode */
enum trx_dict_op_t trx_get_dict_operation(
    const trx_t *trx) /*!< in: transaction */
    MY_ATTRIBUTE((warn_unused_result));
