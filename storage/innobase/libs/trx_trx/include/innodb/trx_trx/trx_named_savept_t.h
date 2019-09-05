#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_savept_t.h>
#include <innodb/lst/lst.h>

/** A savepoint set with SQL's "SAVEPOINT savepoint_id" command */
struct trx_named_savept_t {
  char *name;          /*!< savepoint name */
  trx_savept_t savept; /*!< the undo number corresponding to
                       the savepoint */
  int64_t mysql_binlog_cache_pos;
  /*!< the MySQL binlog cache position
  corresponding to this savepoint, not
  defined if the MySQL binlogging is not
  enabled */
  UT_LIST_NODE_T(trx_named_savept_t)
  trx_savepoints; /*!< the list of savepoints of a
                  transaction */
};
