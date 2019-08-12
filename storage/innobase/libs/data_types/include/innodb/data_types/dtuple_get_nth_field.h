#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

#ifdef UNIV_DEBUG

/** Gets nth field of a tuple.
@param[in]	tuple	tuple
@param[in]	n	index of field
@return nth field */
UNIV_INLINE
dfield_t *dtuple_get_nth_field(const dtuple_t *tuple, ulint n);

#else /* UNIV_DEBUG */

#define dtuple_get_nth_field(tuple, n) ((tuple)->fields + (n))

#endif /* UNIV_DEBUG */
