#include <innodb/data_types/dtype_get_prtype.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/dtype_t.h>

/** Gets the precise data type.
 @return precise data type */
ulint dtype_get_prtype(const dtype_t *type) /*!< in: data type */
{
  ut_ad(type);

  return (type->prtype);
}
