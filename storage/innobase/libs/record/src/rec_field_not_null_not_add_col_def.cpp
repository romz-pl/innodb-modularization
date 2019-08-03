#include <innodb/record/rec_field_not_null_not_add_col_def.h>

/** Determine if the field is not NULL and not having default value
after instant ADD COLUMN
@param[in]	len	length of a field
@return true if not NULL and not having default value */
bool rec_field_not_null_not_add_col_def(ulint len) {
  return (len != UNIV_SQL_NULL && len != UNIV_SQL_ADD_COL_DEFAULT);
}
