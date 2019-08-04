#include <innodb/data_types/dtype_get_mbminlen.h>

#include <innodb/data_types/dtype_t.h>
#include <innodb/assert/assert.h>
#include <innodb/data_types/flags.h>

#ifndef UNIV_HOTBACKUP

/** Gets the minimum length of a character, in bytes.
 @return minimum length of a char, in bytes, or 0 if this is not a
 character type */
ulint dtype_get_mbminlen(const dtype_t *type) /*!< in: type */
{
  ut_ad(type);
  return (DATA_MBMINLEN(type->mbminmaxlen));
}

#endif
