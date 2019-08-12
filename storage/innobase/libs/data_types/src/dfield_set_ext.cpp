#include <innodb/data_types/dfield_set_ext.h>

#include <innodb/data_types/dfield_t.h>

/** Sets the "external storage" flag */
void dfield_set_ext(dfield_t *field) /*!< in/out: field */
{
  ut_ad(field);

  field->ext = 1;
}
