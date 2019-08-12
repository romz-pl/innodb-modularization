#include <innodb/data_types/dfield_copy_data.h>

#include <innodb/data_types/dfield_t.h>


/** Copies the data and len fields. */
void dfield_copy_data(dfield_t *field1,       /*!< out: field to copy to */
                      const dfield_t *field2) /*!< in: field to copy from */
{
  ut_ad(field1 != NULL);
  ut_ad(field2 != NULL);

  field1->data = field2->data;
  field1->len = field2->len;
  field1->ext = field2->ext;
  field1->spatial_status = field2->spatial_status;
}
