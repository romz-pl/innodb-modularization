#include <innodb/data_types/dtype_validate.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/dtype_get_mbmaxlen.h>
#include <innodb/data_types/dtype_get_mbminlen.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/flags.h>

/** Validates a data type structure.
 @return true if ok */
ibool dtype_validate(const dtype_t *type) /*!< in: type struct to validate */
{
  ut_a(type);
  ut_a(type->mtype >= DATA_VARCHAR);
  ut_a(type->mtype <= DATA_MTYPE_MAX);

  if (type->mtype == DATA_SYS) {
    ut_a((type->prtype & DATA_MYSQL_TYPE_MASK) < DATA_N_SYS_COLS);
  }

  ut_a(dtype_get_mbminlen(type) <= dtype_get_mbmaxlen(type));

  return (TRUE);
}
