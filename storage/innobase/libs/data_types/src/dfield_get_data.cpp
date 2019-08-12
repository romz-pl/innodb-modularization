#include <innodb/data_types/dfield_get_data.h>

#include <innodb/data_types/dfield_t.h>

#ifdef UNIV_DEBUG

/** Gets pointer to the data in a field.
 @return pointer to data */
void *dfield_get_data(const dfield_t *field) /*!< in: field */
{
  ut_ad(field);
  ut_ad((field->len == UNIV_SQL_NULL) || (field->data != &data_error));

  return ((void *)field->data);
}

#endif /* UNIV_DEBUG */
