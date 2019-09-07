#pragma once

#include <innodb/univ/univ.h>

/** This is the purge pointer/iterator. We need both the undo no and the
transaction no up to which purge has parsed and applied the records. */
struct purge_iter_t {
  purge_iter_t() : trx_no(), undo_no(), undo_rseg_space(SPACE_UNKNOWN) {
    // Do nothing
  }

  trx_id_t trx_no;   /*!< Purge has advanced past all
                     transactions whose number is less
                     than this */
  undo_no_t undo_no; /*!< Purge has advanced past all records
                     whose undo number is less than this */
  space_id_t undo_rseg_space;
  /*!< Last undo record resided in this
  space id. */
  trx_id_t modifier_trx_id;
  /*!< the transaction that created the
  undo log record. Modifier trx id.*/
};
