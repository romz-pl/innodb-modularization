#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Compare two data tuples.
@param[in] tuple1 first data tuple
@param[in] tuple2 second data tuple
@return whether tuple1==tuple2 */
bool dtuple_coll_eq(const dtuple_t *tuple1, const dtuple_t *tuple2)
    MY_ATTRIBUTE((warn_unused_result));
