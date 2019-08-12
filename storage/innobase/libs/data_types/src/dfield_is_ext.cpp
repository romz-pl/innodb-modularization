#include <innodb/data_types/dfield_is_ext.h>

#include <innodb/data_types/dfield_t.h>

/** Determines if a field is externally stored
 @return nonzero if externally stored */
ulint dfield_is_ext(const dfield_t *field) /*!< in: field */
{
  ut_ad(field);
  ut_ad(!field->ext || field->len >= BTR_EXTERN_FIELD_REF_SIZE);

  return (field->ext);
}
