#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct lock_t;
struct dict_index_t;

/** Checks if some transaction has an implicit x-lock on a record in a clustered
 index.
 @return transaction id of the transaction which has the x-lock, or 0 */
UNIV_INLINE
trx_id_t lock_clust_rec_some_has_impl(
    const rec_t *rec,          /*!< in: user record */
    const dict_index_t *index, /*!< in: clustered index */
    const ulint *offsets)      /*!< in: rec_get_offsets(rec, index) */
    MY_ATTRIBUTE((warn_unused_result));
