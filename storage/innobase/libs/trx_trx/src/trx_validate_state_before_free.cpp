#include <innodb/trx_trx/trx_validate_state_before_free.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/assert_trx_is_inactive.h>
#include <innodb/print/ut_print_buf.h>
#include <innodb/trx_trx/trx_get_id_for_print.h>

void srv_conc_force_exit_innodb(trx_t *trx);

void trx_print(FILE *f, const trx_t *trx, ulint max_query_len);

/** Check state of transaction before freeing it.
@param[in,out]	trx	transaction object to validate */
void trx_validate_state_before_free(trx_t *trx) {
  if (trx->declared_to_be_inside_innodb) {
    ib::error(ER_IB_MSG_1202)
        << "Freeing a trx (" << trx << ", " << trx_get_id_for_print(trx)
        << ") which is declared"
           " to be processing inside InnoDB";

    trx_print(stderr, trx, 600);
    putc('\n', stderr);

    /* This is an error but not a fatal error. We must keep
    the counters like srv_conc_n_threads accurate. */
    srv_conc_force_exit_innodb(trx);
  }

  if (trx->n_mysql_tables_in_use != 0 || trx->mysql_n_tables_locked != 0) {
    ib::error(ER_IB_MSG_1203)
        << "MySQL is freeing a thd though"
           " trx->n_mysql_tables_in_use is "
        << trx->n_mysql_tables_in_use << " and trx->mysql_n_tables_locked is "
        << trx->mysql_n_tables_locked << ".";

    trx_print(stderr, trx, 600);
    ut_print_buf(stderr, trx, sizeof(trx_t));
    putc('\n', stderr);
  }

  trx->dict_operation = TRX_DICT_OP_NONE;
  assert_trx_is_inactive(trx);
}
