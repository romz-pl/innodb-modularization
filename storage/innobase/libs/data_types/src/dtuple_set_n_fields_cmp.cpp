#include <innodb/data_types/dtuple_set_n_fields_cmp.h>

#include <innodb/data_types/dtuple_t.h>

/** Sets number of fields used in record comparisons. */
void dtuple_set_n_fields_cmp(
    dtuple_t *tuple,    /*!< in: tuple */
    ulint n_fields_cmp) /*!< in: number of fields used in
                        comparisons in rem0cmp.* */
{
  ut_ad(tuple);
  ut_ad(n_fields_cmp <= tuple->n_fields);

  tuple->n_fields_cmp = n_fields_cmp;
}
