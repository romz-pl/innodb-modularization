#include <innodb/data_types/dtuple_set_types_binary.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtype_set.h>
#include <innodb/data_types/flags.h>

/** Sets types of fields binary in a tuple. */
void dtuple_set_types_binary(dtuple_t *tuple, /*!< in: data tuple */
                             ulint n) /*!< in: number of fields to set */
{
  dtype_t *dfield_type;
  ulint i;

  for (i = 0; i < n; i++) {
    dfield_type = dfield_get_type(dtuple_get_nth_field(tuple, i));
    dtype_set(dfield_type, DATA_BINARY, 0, 0);
  }
}
