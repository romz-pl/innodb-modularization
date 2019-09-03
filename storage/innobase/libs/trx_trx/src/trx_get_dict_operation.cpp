#include <innodb/trx_trx/trx_get_dict_operation.h>

#include <innodb/trx_trx/trx_t.h>

/** Determine if a transaction is a dictionary operation.
 @return dictionary operation mode */
enum trx_dict_op_t trx_get_dict_operation(
    const trx_t *trx) /*!< in: transaction */
{
  trx_dict_op_t op = static_cast<trx_dict_op_t>(trx->dict_operation);

#ifdef UNIV_DEBUG
  switch (op) {
    case TRX_DICT_OP_NONE:
    case TRX_DICT_OP_TABLE:
    case TRX_DICT_OP_INDEX:
      return (op);
  }
  ut_error;
#endif /* UNIV_DEBUG */
  return (op);
}
