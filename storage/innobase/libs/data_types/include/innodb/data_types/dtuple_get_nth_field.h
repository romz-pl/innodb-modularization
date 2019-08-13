#pragma once

#include <innodb/univ/univ.h>


#ifdef UNIV_DEBUG

struct dtuple_t;

/** Gets nth field of a tuple.
@param[in]	tuple	tuple
@param[in]	n	index of field
@return nth field */
dfield_t *dtuple_get_nth_field(const dtuple_t *tuple, ulint n);

#else /* UNIV_DEBUG */

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dfield_t.h>

#define dtuple_get_nth_field(tuple, n) ((tuple)->fields + (n))

#endif /* UNIV_DEBUG */
