#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Checks that a data tuple is typed. Asserts an error if not.
 @return true if ok */
ibool dtuple_check_typed(const dtuple_t *tuple) /*!< in: tuple */
    MY_ATTRIBUTE((warn_unused_result));
