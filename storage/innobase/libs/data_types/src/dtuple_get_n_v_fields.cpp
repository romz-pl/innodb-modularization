#include <innodb/data_types/dtuple_get_n_v_fields.h>

#include <innodb/data_types/dtuple_t.h>

/** Gets the number of virtual fields in a data tuple.
@param[in]	tuple	dtuple to check
@return number of fields */
ulint dtuple_get_n_v_fields(const dtuple_t *tuple) {
  ut_ad(tuple);

  return (tuple->n_v_fields);
}
