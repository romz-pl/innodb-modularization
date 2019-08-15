#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;

/** Gets the number of fields in the internal representation of an index,
 including fields added by the dictionary system.
 @return number of fields */
ulint dict_index_get_n_fields(
    const dict_index_t *index) /*!< in: an internal
                               representation of index (in
                               the dictionary cache) */
    MY_ATTRIBUTE((warn_unused_result));
