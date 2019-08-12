#include <innodb/data_types/dfield_set_type.h>

#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dtype_t.h>

/** Sets the type struct of SQL data field. */
void dfield_set_type(
    dfield_t *field,     /*!< in: SQL data field */
    const dtype_t *type) /*!< in: pointer to data type struct */
{
  ut_ad(field != NULL);
  ut_ad(type != NULL);

  field->type = *type;
}
