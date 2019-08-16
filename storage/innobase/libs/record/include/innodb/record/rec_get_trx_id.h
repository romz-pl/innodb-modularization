#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;

/** Reads the DB_TRX_ID of a clustered index record.
 @return the value of DB_TRX_ID */
trx_id_t rec_get_trx_id(const rec_t *rec,          /*!< in: record */
                        const dict_index_t *index) /*!< in: clustered index */
    MY_ATTRIBUTE((warn_unused_result));
