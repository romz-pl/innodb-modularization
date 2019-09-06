#pragma once

#include <innodb/univ/univ.h>

#include "sql/handler.h"

struct trx_t;

/** This function is used to find number of prepared transactions and
 their transaction objects for a recovery.
 @return number of prepared transactions */
int trx_recover_for_mysql(
    XA_recover_txn *txn_list, /*!< in/out: prepared transactions */
    ulint len,                /*!< in: number of slots in xid_list */
    MEM_ROOT *mem_root);      /*!< in: memory for table names */
