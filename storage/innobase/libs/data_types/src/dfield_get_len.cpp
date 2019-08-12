#include <innodb/data_types/dfield_get_len.h>

#include <innodb/data_types/dfield_t.h>

/** Gets length of field data.
 @return length of data; UNIV_SQL_NULL if SQL null data */
ulint dfield_get_len(const dfield_t *field) /*!< in: field */
{
  ut_ad(field);
  ut_ad((field->len == UNIV_SQL_NULL) || (field->data != &data_error));

  return (field->len);
}
