#include <innodb/data_types/dtype_get_mbmaxlen.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/flags.h>
#include <innodb/assert/assert.h>

/** Gets the maximum length of a character, in bytes.
 @return maximum length of a char, in bytes, or 0 if this is not a
 character type */
ulint dtype_get_mbmaxlen(const dtype_t *type) /*!< in: type */
{
  ut_ad(type);
  return (DATA_MBMAXLEN(type->mbminmaxlen));
}

#endif
