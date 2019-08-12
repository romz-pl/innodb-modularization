#include <innodb/data_types/dfield_set_null.h>

#include <innodb/data_types/dfield_set_data.h>


/** Sets a data field to SQL NULL. */
void dfield_set_null(dfield_t *field) /*!< in/out: field */
{
  dfield_set_data(field, NULL, UNIV_SQL_NULL);
}
