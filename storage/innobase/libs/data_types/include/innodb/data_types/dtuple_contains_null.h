#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Checks if a dtuple contains an SQL null value.
 @return true if some field is SQL null */
ibool dtuple_contains_null(const dtuple_t *tuple) /*!< in: dtuple */
    MY_ATTRIBUTE((warn_unused_result));
