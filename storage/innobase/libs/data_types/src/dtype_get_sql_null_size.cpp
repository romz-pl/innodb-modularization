#include <innodb/data_types/dtype_get_sql_null_size.h>

#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/dtype_get_fixed_size_low.h>
#include <innodb/data_types/dtype_get_fixed_size_low.h>

/** Returns the ROW_FORMAT=REDUNDANT stored SQL NULL size of a type.
 For fixed length types it is the fixed length of the type, otherwise 0.
 @return SQL null storage size in ROW_FORMAT=REDUNDANT */
ulint dtype_get_sql_null_size(const dtype_t *type, /*!< in: type */
                              ulint comp) /*!< in: nonzero=ROW_FORMAT=COMPACT */
{
#ifndef UNIV_HOTBACKUP
  return (dtype_get_fixed_size_low(type->mtype, type->prtype, type->len,
                                   type->mbminmaxlen, comp));
#else  /* !UNIV_HOTBACKUP */
  return (dtype_get_fixed_size_low(type->mtype, type->prtype, type->len, 0, 0));
#endif /* !UNIV_HOTBACKUP */
}
