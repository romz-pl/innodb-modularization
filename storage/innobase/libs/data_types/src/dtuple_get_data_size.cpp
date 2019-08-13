#include <innodb/data_types/dtuple_get_data_size.h>

#include <innodb/data_types/dtuple_check_typed.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dtype_get_sql_null_size.h>
#include <innodb/data_types/dfield_get_type.h>

/** The following function returns the sum of data lengths of a tuple. The space
 occupied by the field structs or the tuple struct is not counted. Neither
 is possible space in externally stored parts of the field.
 @return sum of data lengths */
ulint dtuple_get_data_size(const dtuple_t *tuple, /*!< in: typed data tuple */
                           ulint comp) /*!< in: nonzero=ROW_FORMAT=COMPACT  */
{
  const dfield_t *field;
  ulint n_fields;
  ulint len;
  ulint i;
  ulint sum = 0;

  ut_ad(tuple);
  ut_ad(dtuple_check_typed(tuple));
  ut_ad(tuple->magic_n == DATA_TUPLE_MAGIC_N);

  n_fields = tuple->n_fields;

  for (i = 0; i < n_fields; i++) {
    field = dtuple_get_nth_field(tuple, i);
    len = dfield_get_len(field);

    if (len == UNIV_SQL_NULL) {
      len = dtype_get_sql_null_size(dfield_get_type(field), comp);
    }

    sum += len;
  }

  return (sum);
}
