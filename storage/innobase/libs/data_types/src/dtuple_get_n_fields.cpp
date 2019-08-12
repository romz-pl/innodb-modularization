#include <innodb/data_types/dtuple_get_n_fields.h>

#include <innodb/data_types/dtuple_t.h>

/** Gets number of fields in a data tuple.
 @return number of fields */
ulint dtuple_get_n_fields(const dtuple_t *tuple) /*!< in: tuple */
{
  ut_ad(tuple);

  return (tuple->n_fields);
}
