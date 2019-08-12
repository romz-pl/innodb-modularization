#include <innodb/data_types/dfield_get_type.h>

#include <innodb/data_types/dfield_t.h>

#ifdef UNIV_DEBUG

/** Gets pointer to the type struct of SQL data field.
 @return pointer to the type struct */

dtype_t *dfield_get_type(const dfield_t *field) /*!< in: SQL data field */
{
  ut_ad(field);

  return ((dtype_t *)&(field->type));
}
#endif /* UNIV_DEBUG */
