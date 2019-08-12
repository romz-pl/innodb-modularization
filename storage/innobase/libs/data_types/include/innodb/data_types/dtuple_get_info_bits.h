#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Gets info bits in a data tuple.
 @return info bits */
ulint dtuple_get_info_bits(const dtuple_t *tuple) /*!< in: tuple */
    MY_ATTRIBUTE((warn_unused_result));
