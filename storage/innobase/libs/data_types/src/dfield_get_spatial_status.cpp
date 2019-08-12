#include <innodb/data_types/dfield_get_spatial_status.h>

#include <innodb/data_types/dfield_t.h>

/** Gets spatial status for "external storage"
@param[in,out]	field		field */
spatial_status_t dfield_get_spatial_status(const dfield_t *field) {
  ut_ad(field);
  ut_ad(dfield_is_ext(field));

  return (static_cast<spatial_status_t>(field->spatial_status));
}
