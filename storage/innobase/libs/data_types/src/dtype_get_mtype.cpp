#include <innodb/data_types/dtype_get_mtype.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/dtype_t.h>

/** Gets the SQL main data type.
 @return SQL main data type */
ulint dtype_get_mtype(const dtype_t *type) /*!< in: data type */
{
  ut_ad(type);

  return (type->mtype);
}
