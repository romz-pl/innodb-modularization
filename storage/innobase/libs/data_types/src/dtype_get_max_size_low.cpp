#include <innodb/data_types/dtype_get_max_size_low.h>

#include <innodb/data_types/flags.h>
#include <innodb/error/ut_error.h>

/** Returns the maximum size of a data type. Note: types in system tables may
be incomplete and return incorrect information.
@param[in]	mtype	main type
@param[in]	len	length
@return maximum size */
ulint dtype_get_max_size_low(ulint mtype, /*!< in: main type */
                             ulint len)   /*!< in: length */
{
  switch (mtype) {
    case DATA_SYS:
    case DATA_CHAR:
    case DATA_FIXBINARY:
    case DATA_INT:
    case DATA_FLOAT:
    case DATA_DOUBLE:
    case DATA_MYSQL:
    case DATA_VARCHAR:
    case DATA_BINARY:
    case DATA_DECIMAL:
    case DATA_VARMYSQL:
    case DATA_POINT:
      return (len);
    case DATA_VAR_POINT:
    case DATA_GEOMETRY:
    case DATA_BLOB:
      break;
    default:
      ut_error;
  }
  return (ULINT_MAX);
}
