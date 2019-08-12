#include <innodb/data_types/dtuple_get_n_fields_cmp.h>

#include <innodb/data_types/dtuple_t.h>

/** Gets number of fields used in record comparisons.
 @return number of fields used in comparisons in rem0cmp.* */
ulint dtuple_get_n_fields_cmp(const dtuple_t *tuple) /*!< in: tuple */
{
  ut_ad(tuple);

  return (tuple->n_fields_cmp);
}
