#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

#ifdef UNIV_DEBUG

/** Validates the consistency of a tuple which must be complete, i.e,
 all fields must have been set.
 @return true if ok */
ibool dtuple_validate(const dtuple_t *tuple) /*!< in: tuple */
    MY_ATTRIBUTE((warn_unused_result));

#endif /* UNIV_DEBUG */
