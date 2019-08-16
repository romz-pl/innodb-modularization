#include <innodb/cmp/cmp_data.h>

#include <innodb/assert/assert.h>
#include <innodb/cmp/cmp_whole_field.h>
#include <innodb/cmp/cmp_whole_field.h>
#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/data_types/flags.h>
#include <innodb/machine/data.h>

#include "include/m_ctype.h"

/** Compare two data fields.
@param[in]	mtype	main type
@param[in]	prtype	precise type
@param[in]	is_asc	true=ascending, false=descending order
@param[in]	data1	data field
@param[in]	len1	length of data1 in bytes, or UNIV_SQL_NULL
@param[in]	data2	data field
@param[in]	len2	length of data2 in bytes, or UNIV_SQL_NULL
@return the comparison result of data1 and data2
@retval 0 if data1 is equal to data2
@retval negative if data1 is less than data2
@retval positive if data1 is greater than data2 */
inline int cmp_data(ulint mtype, ulint prtype, bool is_asc, const byte *data1,
                    ulint len1, const byte *data2, ulint len2) {
  if (len1 == UNIV_SQL_NULL || len2 == UNIV_SQL_NULL) {
    if (len1 == len2) {
      return (0);
    }

    /* We define the SQL null to be the smallest possible
    value of a field. */
    return ((len1 == UNIV_SQL_NULL) == is_asc ? -1 : 1);
  }

  ulint pad;

  switch (mtype) {
    case DATA_FIXBINARY:
    case DATA_BINARY:
      if (dtype_get_charset_coll(prtype) != DATA_MYSQL_BINARY_CHARSET_COLL) {
        pad = 0x20;
        break;
      }
      /* fall through */
    case DATA_INT:
    case DATA_SYS_CHILD:
    case DATA_SYS:
      pad = ULINT_UNDEFINED;
      break;
    case DATA_POINT:
    case DATA_VAR_POINT:
      /* Since DATA_POINT has a fixed length of DATA_POINT_LEN,
      currently, pad is not needed. Meanwhile, DATA_VAR_POINT acts
      the same as DATA_GEOMETRY */
    case DATA_GEOMETRY:
      ut_ad(prtype & DATA_BINARY_TYPE);
      pad = ULINT_UNDEFINED;
      if (prtype & DATA_GIS_MBR) {
        return (cmp_whole_field(mtype, prtype, is_asc, data1, (unsigned)len1,
                                data2, (unsigned)len2));
      }
      break;
    case DATA_BLOB:
      if (prtype & DATA_BINARY_TYPE) {
        pad = ULINT_UNDEFINED;
        break;
      }
      /* fall through */
    default:
      return (cmp_whole_field(mtype, prtype, is_asc, data1, (unsigned)len1,
                              data2, (unsigned)len2));
  }

  ulint len;
  int cmp;

  if (len1 < len2) {
    len = len1;
    len2 -= len;
    len1 = 0;
  } else {
    len = len2;
    len1 -= len;
    len2 = 0;
  }

  if (len) {
#if defined __i386__ || defined __x86_64__ || defined _M_IX86 || defined _M_X64
    /* Compare the first bytes with a loop to avoid the call
    overhead of memcmp(). On x86 and x86-64, the GCC built-in
    (repz cmpsb) seems to be very slow, so we will be calling the
    libc version. http://gcc.gnu.org/bugzilla/show_bug.cgi?id=43052
    tracks the slowness of the GCC built-in memcmp().

    We compare up to the first 4..7 bytes with the loop.
    The (len & 3) is used for "normalizing" or
    "quantizing" the len parameter for the memcmp() call,
    in case the whole prefix is equal. On x86 and x86-64,
    the GNU libc memcmp() of equal strings is faster with
    len=4 than with len=3.

    On other architectures than the IA32 or AMD64, there could
    be a built-in memcmp() that is faster than the loop.
    We only use the loop where we know that it can improve
    the performance. */
    for (ulint i = 4 + (len & 3); i > 0; i--) {
      cmp = int(*data1++) - int(*data2++);
      if (cmp) {
        goto func_exit;
      }

      if (!--len) {
        break;
      }
    }

    if (len) {
#endif /* IA32 or AMD64 */
      cmp = memcmp(data1, data2, len);

      if (cmp) {
        goto func_exit;
      }

      data1 += len;
      data2 += len;
#if defined __i386__ || defined __x86_64__ || defined _M_IX86 || defined _M_X64
    }
#endif /* IA32 or AMD64 */
  }

  cmp = (int)(len1 - len2);

  if (!cmp) {
    return (cmp);
  }

  if (pad == ULINT_UNDEFINED) {
    goto func_exit;
  }

  len = 0;

  if (len1) {
    do {
      cmp = static_cast<int>(mach_read_from_1(&data1[len++]) - pad);
    } while (cmp == 0 && len < len1);
  } else {
    ut_ad(len2 > 0);

    do {
      cmp = static_cast<int>(pad - mach_read_from_1(&data2[len++]));
    } while (cmp == 0 && len < len2);
  }

func_exit:
  return (is_asc ? cmp : -cmp);
}
