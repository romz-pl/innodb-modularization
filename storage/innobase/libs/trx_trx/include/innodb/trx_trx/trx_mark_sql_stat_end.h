#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Marks the latest SQL statement ended. */
void trx_mark_sql_stat_end(trx_t *trx); /*!< in: trx handle */
