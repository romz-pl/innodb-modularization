#include <innodb/data_types/dtuple_contains_null.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dfield_is_null.h>

/** Checks if a dtuple contains an SQL null value.
 @return true if some field is SQL null */
ibool dtuple_contains_null(const dtuple_t *tuple) /*!< in: dtuple */
{
  ulint n;
  ulint i;

  n = dtuple_get_n_fields(tuple);

  for (i = 0; i < n; i++) {
    if (dfield_is_null(dtuple_get_nth_field(tuple, i))) {
      return (TRUE);
    }
  }

  return (FALSE);
}
