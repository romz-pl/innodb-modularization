#include <innodb/data_types/dtuple_get_nth_field.h>


#ifdef UNIV_DEBUG

/** Gets nth field of a tuple.
@param[in]	tuple	tuple
@param[in]	n	index of field
@return nth field */
UNIV_INLINE
dfield_t *dtuple_get_nth_field(const dtuple_t *tuple, ulint n) {
  ut_ad(tuple);
  ut_ad(n < tuple->n_fields);

  return ((dfield_t *)tuple->fields + n);
}

#endif /* UNIV_DEBUG */
