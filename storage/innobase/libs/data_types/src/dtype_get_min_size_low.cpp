#include <innodb/data_types/dtype_get_min_size_low.h>

#include <innodb/data_types/flags.h>
#include <innodb/assert/assert.h>
#include <innodb/error/ut_error.h>

/** Returns the minimum size of a data type.
 @return minimum size */
ulint dtype_get_min_size_low(
    ulint mtype,       /*!< in: main type */
    ulint prtype,      /*!< in: precise type */
    ulint len,         /*!< in: length */
    ulint mbminmaxlen) /*!< in: minimum and maximum length of a
                       multi-byte character */
{
  switch (mtype) {
    case DATA_SYS:
#ifdef UNIV_DEBUG
      switch (prtype & DATA_MYSQL_TYPE_MASK) {
        case DATA_ROW_ID:
          ut_ad(len == DATA_ROW_ID_LEN);
          break;
        case DATA_TRX_ID:
          ut_ad(len == DATA_TRX_ID_LEN);
          break;
        case DATA_ROLL_PTR:
          ut_ad(len == DATA_ROLL_PTR_LEN);
          break;
        default:
          ut_ad(0);
          return (0);
      }
#endif /* UNIV_DEBUG */
    // Fall through.
    case DATA_CHAR:
    case DATA_FIXBINARY:
    case DATA_INT:
    case DATA_FLOAT:
    case DATA_DOUBLE:
    case DATA_POINT:
      return (len);
    case DATA_MYSQL:
      if (prtype & DATA_BINARY_TYPE) {
        return (len);
      } else {
        ulint mbminlen = DATA_MBMINLEN(mbminmaxlen);
        ulint mbmaxlen = DATA_MBMAXLEN(mbminmaxlen);

        if (mbminlen == mbmaxlen) {
          return (len);
        }

        /* this is a variable-length character set */
        ut_a(mbminlen > 0);
        ut_a(mbmaxlen > mbminlen);
        ut_a(len % mbmaxlen == 0);
        return (len * mbminlen / mbmaxlen);
      }
    case DATA_VARCHAR:
    case DATA_BINARY:
    case DATA_DECIMAL:
    case DATA_VARMYSQL:
    case DATA_VAR_POINT:
    case DATA_GEOMETRY:
    case DATA_BLOB:
      return (0);
    default:
      ut_error;
  }
}
