#include <innodb/data_types/dtuple_get_n_ext.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_check_typed.h>
#include <innodb/data_types/dtuple_get_nth_field.h>

/** Computes the number of externally stored fields in a data tuple.
 @return number of externally stored fields */
ulint dtuple_get_n_ext(const dtuple_t *tuple) /*!< in: tuple */
{
  ulint n_ext = 0;
  ulint n_fields = tuple->n_fields;
  ulint i;

  ut_ad(tuple);
  ut_ad(dtuple_check_typed(tuple));
  ut_ad(tuple->magic_n == DATA_TUPLE_MAGIC_N);

  for (i = 0; i < n_fields; i++) {
    n_ext += dtuple_get_nth_field(tuple, i)->ext;
  }

  return (n_ext);
}
