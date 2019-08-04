#include <innodb/data_types/dtype_get_len.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/dtype_t.h>

/** Gets the type length.
 @return fixed length of the type, in bytes, or 0 if variable-length */
ulint dtype_get_len(const dtype_t *type) /*!< in: data type */
{
  ut_ad(type);

  return (type->len);
}
