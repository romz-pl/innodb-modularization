#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Gets number of fields in a data tuple.
 @return number of fields */
ulint dtuple_get_n_fields(const dtuple_t *tuple) /*!< in: tuple */
    MY_ATTRIBUTE((warn_unused_result));
