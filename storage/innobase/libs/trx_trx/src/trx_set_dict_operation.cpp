#include <innodb/trx_trx/trx_set_dict_operation.h>

#include <innodb/trx_trx/trx_t.h>

/** Flag a transaction a dictionary operation. */
void trx_set_dict_operation(trx_t *trx,            /*!< in/out: transaction */
                            enum trx_dict_op_t op) /*!< in: operation, not
                                                   TRX_DICT_OP_NONE */
{
#ifdef UNIV_DEBUG
  enum trx_dict_op_t old_op = trx_get_dict_operation(trx);

  switch (op) {
    case TRX_DICT_OP_NONE:
      ut_error;
      break;
    case TRX_DICT_OP_TABLE:
      switch (old_op) {
        case TRX_DICT_OP_NONE:
        case TRX_DICT_OP_INDEX:
        case TRX_DICT_OP_TABLE:
          goto ok;
      }
      ut_error;
      break;
    case TRX_DICT_OP_INDEX:
      ut_ad(old_op == TRX_DICT_OP_NONE);
      break;
  }
ok:
#endif /* UNIV_DEBUG */

  trx->dict_operation = op;
}
