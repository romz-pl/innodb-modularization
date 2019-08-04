#include <innodb/data_types/dtype_get_fixed_size_low.h>

#include <innodb/data_types/flags.h>
#include <innodb/error/ut_error.h>

/** Returns the size of a fixed size data type, 0 if not a fixed size type.
 @return fixed size, or 0 */
ulint dtype_get_fixed_size_low(
    ulint mtype,       /*!< in: main type */
    ulint prtype,      /*!< in: precise type */
    ulint len,         /*!< in: length */
    ulint mbminmaxlen, /*!< in: minimum and maximum length of
                       a multibyte character, in bytes */
    ulint comp)        /*!< in: nonzero=ROW_FORMAT=COMPACT  */
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
#ifndef UNIV_LIBRARY
      if (prtype & DATA_BINARY_TYPE) {
        return (len);
      } else if (!comp) {
        return (len);
      } else {
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG
        ulint i_mbminlen, i_mbmaxlen;

        innobase_get_cset_width(dtype_get_charset_coll(prtype), &i_mbminlen,
                                &i_mbmaxlen);

        ut_ad(DATA_MBMINMAXLEN(i_mbminlen, i_mbmaxlen) == mbminmaxlen);
#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */
        if (DATA_MBMINLEN(mbminmaxlen) == DATA_MBMAXLEN(mbminmaxlen)) {
          return (len);
        }
      }
#else  /* !UNIV_LIBRARY */
      return (len);
#endif /* !UNIV_LIBRARY */
       /* fall through for variable-length charsets */
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
