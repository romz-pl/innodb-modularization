#include <innodb/data_types/dtype_set_mbminmaxlen.h>

#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/flags.h>
#include <innodb/assert/assert.h>

#ifndef UNIV_HOTBACKUP

/** Sets the minimum and maximum length of a character, in bytes. */
void dtype_set_mbminmaxlen(dtype_t *type,  /*!< in/out: type */
                           ulint mbminlen, /*!< in: minimum length of a char,
                                           in bytes, or 0 if this is not
                                           a character type */
                           ulint mbmaxlen) /*!< in: maximum length of a char,
                                           in bytes, or 0 if this is not
                                           a character type */
{
  ut_ad(mbminlen < DATA_MBMAX);
  ut_ad(mbmaxlen < DATA_MBMAX);
  ut_ad(mbminlen <= mbmaxlen);

  type->mbminmaxlen = DATA_MBMINMAXLEN(mbminlen, mbmaxlen);
}

#endif
