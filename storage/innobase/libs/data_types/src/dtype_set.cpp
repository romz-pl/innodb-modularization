#include <innodb/data_types/dtype_set.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/dtype_set_mblen.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/flags.h>

/** Sets a data type structure. */
void dtype_set(dtype_t *type, /*!< in: type struct to init */
               ulint mtype,   /*!< in: main data type */
               ulint prtype,  /*!< in: precise type */
               ulint len)     /*!< in: precision of type */
{
  ut_ad(type);
  ut_ad(mtype <= DATA_MTYPE_MAX);

  type->mtype = mtype;
  type->prtype = prtype;
  type->len = len;

  dtype_set_mblen(type);
}
