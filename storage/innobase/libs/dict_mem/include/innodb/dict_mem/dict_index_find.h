#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;
class index_id_t;

/** Look up an index.
@param[in]	id	index identifier
@return index or NULL if not found */
const dict_index_t *dict_index_find(const index_id_t &id)
    MY_ATTRIBUTE((warn_unused_result));
