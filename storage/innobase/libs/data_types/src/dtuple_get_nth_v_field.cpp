#include <innodb/data_types/dtuple_get_nth_v_field.h>


#ifdef UNIV_DEBUG

#include <innodb/data_types/dtuple_t.h>

/** Gets nth virtual field of a tuple.
@param[in]	tuple	tuple
@oaran[in]	n	the nth field to get
@return nth field */
UNIV_INLINE
dfield_t *dtuple_get_nth_v_field(const dtuple_t *tuple, ulint n) {
  ut_ad(tuple);
  ut_ad(n < tuple->n_v_fields);

  return (static_cast<dfield_t *>(tuple->v_fields + n));
}

#endif /* UNIV_DEBUG */
