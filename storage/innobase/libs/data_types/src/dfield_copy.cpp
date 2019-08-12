#include <innodb/data_types/dfield_copy.h>

#include <innodb/data_types/dfield_t.h>

/** Copies a data field to another. */
void dfield_copy(dfield_t *field1,       /*!< out: field to copy to */
                 const dfield_t *field2) /*!< in: field to copy from */
{
  *field1 = *field2;
}
