#include <innodb/data_types/dfield_set_spatial_status.h>

#include <innodb/data_types/dfield_t.h>

/** Sets spatial status for "external storage"
@param[in,out]	field		field
@param[in]	spatial_status	spatial status */
void dfield_set_spatial_status(dfield_t *field,
                               spatial_status_t spatial_status) {
  ut_ad(field);
  ut_ad(dfield_is_ext(field));

  field->spatial_status = spatial_status;
}
