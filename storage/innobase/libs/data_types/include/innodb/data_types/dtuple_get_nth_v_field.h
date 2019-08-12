#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

#ifdef UNIV_DEBUG

/** Gets nth virtual field of a tuple.
@param[in]	tuple	tuple
@param[in]	n	the nth field to get
@return nth field */
UNIV_INLINE
dfield_t *dtuple_get_nth_v_field(const dtuple_t *tuple, ulint n);

#else /* UNIV_DEBUG */

#define dtuple_get_nth_v_field(tuple, n) \
  ((tuple)->fields + (tuple)->n_fields + (n))

#endif /* UNIV_DEBUG */
