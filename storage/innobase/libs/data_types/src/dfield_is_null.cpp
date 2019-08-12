#include <innodb/data_types/dfield_is_null.h>

#include <innodb/data_types/dfield_t.h>

/** Determines if a field is SQL NULL
 @return nonzero if SQL null data */
ulint dfield_is_null(const dfield_t *field) /*!< in: field */
{
  ut_ad(field);

  return (field->len == UNIV_SQL_NULL);
}
