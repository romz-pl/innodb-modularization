#pragma once

#include <innodb/univ/univ.h>

/** Type of data dictionary operation */
enum trx_dict_op_t {
  /** The transaction is not modifying the data dictionary. */
  TRX_DICT_OP_NONE = 0,
  /** The transaction is creating a table or an index, or
  dropping a table.  The table must be dropped in crash
  recovery.  This and TRX_DICT_OP_NONE are the only possible
  operation modes in crash recovery. */
  TRX_DICT_OP_TABLE = 1,
  /** The transaction is creating or dropping an index in an
  existing table.  In crash recovery, the data dictionary
  must be locked, but the table must not be dropped. */
  TRX_DICT_OP_INDEX = 2
};
