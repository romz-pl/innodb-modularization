#include <innodb/data_types/dtuple_set_n_fields.h>

#include <innodb/data_types/dtuple_t.h>

/** Sets number of fields used in a tuple. Normally this is set in
 dtuple_create, but if you want later to set it smaller, you can use this. */
void dtuple_set_n_fields(dtuple_t *tuple, /*!< in: tuple */
                         ulint n_fields)  /*!< in: number of fields */
{
  ut_ad(tuple);

  tuple->n_fields = n_fields;
  tuple->n_fields_cmp = n_fields;
}
