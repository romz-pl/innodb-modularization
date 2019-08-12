#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Gets number of fields used in record comparisons.
 @return number of fields used in comparisons in rem0cmp.* */
ulint dtuple_get_n_fields_cmp(const dtuple_t *tuple) /*!< in: tuple */
    MY_ATTRIBUTE((warn_unused_result));
