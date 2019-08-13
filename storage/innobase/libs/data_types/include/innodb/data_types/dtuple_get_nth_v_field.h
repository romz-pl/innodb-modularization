#pragma once

#include <innodb/univ/univ.h>


#ifdef UNIV_DEBUG

struct dtuple_t;

/** Gets nth virtual field of a tuple.
@param[in]	tuple	tuple
@param[in]	n	the nth field to get
@return nth field */
UNIV_INLINE
dfield_t *dtuple_get_nth_v_field(const dtuple_t *tuple, ulint n);

#else /* UNIV_DEBUG */

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dfield_t.h>

#define dtuple_get_nth_v_field(tuple, n) \
  ((tuple)->fields + (tuple)->n_fields + (n))

#endif /* UNIV_DEBUG */
