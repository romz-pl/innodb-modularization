#include <innodb/data_types/dtype_get_mblen.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/data_types/dtype_is_string_type.h>
#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/data_types/innobase_get_cset_width.h>
#include <innodb/assert/assert.h>

/** Compute the mbminlen and mbmaxlen members of a data type structure. */
void dtype_get_mblen(ulint mtype,     /*!< in: main type */
                     ulint prtype,    /*!< in: precise type (and collation) */
                     ulint *mbminlen, /*!< out: minimum length of a
                                      multi-byte character */
                     ulint *mbmaxlen) /*!< out: maximum length of a
                                      multi-byte character */
{
  if (dtype_is_string_type(mtype)) {
    innobase_get_cset_width(dtype_get_charset_coll(prtype), mbminlen, mbmaxlen);
    ut_ad(*mbminlen <= *mbmaxlen);
    ut_ad(*mbminlen < DATA_MBMAX);
    ut_ad(*mbmaxlen < DATA_MBMAX);
  } else {
    *mbminlen = *mbmaxlen = 0;
  }
}

#endif
