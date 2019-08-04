#include <innodb/data_types/dtype_set_mblen.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/dtype_get_mblen.h>
#include <innodb/data_types/dtype_set_mbminmaxlen.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/dtype_validate.h>

#ifndef UNIV_HOTBACKUP

/** Compute the mbminlen and mbmaxlen members of a data type structure. */
void dtype_set_mblen(dtype_t *type) /*!< in/out: type */
{
  ulint mbminlen;
  ulint mbmaxlen;

  dtype_get_mblen(type->mtype, type->prtype, &mbminlen, &mbmaxlen);
  dtype_set_mbminmaxlen(type, mbminlen, mbmaxlen);

  ut_ad(dtype_validate(type));
}

#endif /* !UNIV_HOTBACKUP */
