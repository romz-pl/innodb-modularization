#pragma once

#include <innodb/univ/univ.h>

struct trx_t;
class ReadView;

/** Assigns a read view for a consistent read query. All the consistent reads
 within the same transaction will get the same read view, which is created
 when this function is first called for a new started transaction. */
ReadView *trx_assign_read_view(trx_t *trx); /*!< in: active transaction */
