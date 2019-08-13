#pragma once

#include <innodb/univ/univ.h>

/** Error to ignore when we load table dictionary into memory. However,
the table and index will be marked as "corrupted", and caller will
be responsible to deal with corrupted table or index.
Note: please define the IGNORE_ERR_* as bits, so their value can
be or-ed together */
enum dict_err_ignore_t {
  DICT_ERR_IGNORE_NONE = 0,       /*!< no error to ignore */
  DICT_ERR_IGNORE_INDEX_ROOT = 1, /*!< ignore error if index root
                                  page is FIL_NULL or incorrect value */
  DICT_ERR_IGNORE_CORRUPT = 2,    /*!< skip corrupted indexes */
  DICT_ERR_IGNORE_FK_NOKEY = 4,   /*!< ignore error if any foreign
                                  key is missing */
  DICT_ERR_IGNORE_RECOVER_LOCK = 8,
  /*!< Used when recovering table locks
  for resurrected transactions.
  Silently load a missing
  tablespace, and do not load
  incomplete index definitions. */
  DICT_ERR_IGNORE_ALL = 0xFFFF /*!< ignore all errors */
};
