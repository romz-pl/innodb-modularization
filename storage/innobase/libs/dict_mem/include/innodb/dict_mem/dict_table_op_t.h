#pragma once

#include <innodb/univ/univ.h>

/** Operation to perform when opening a table */
enum dict_table_op_t {
  /** Expect the tablespace to exist. */
  DICT_TABLE_OP_NORMAL = 0,
  /** Drop any orphan indexes after an aborted online index creation */
  DICT_TABLE_OP_DROP_ORPHAN,
  /** Silently load the tablespace if it does not exist,
  and do not load the definitions of incomplete indexes. */
  DICT_TABLE_OP_LOAD_TABLESPACE
};
