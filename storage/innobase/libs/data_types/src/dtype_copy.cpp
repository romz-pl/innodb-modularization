#include <innodb/data_types/dtype_copy.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/dtype_validate.h>

/** Copies a data type structure. */
void dtype_copy(dtype_t *type1,       /*!< in: type struct to copy to */
                const dtype_t *type2) /*!< in: type struct to copy from */
{
  *type1 = *type2;

  ut_ad(dtype_validate(type1));
}
