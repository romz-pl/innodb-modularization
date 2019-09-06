#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** If required, flushes the log to disk if we called trx_commit_for_mysql()
 with trx->flush_log_later == TRUE. */
void trx_commit_complete_for_mysql(trx_t *trx); /*!< in/out: transaction */
