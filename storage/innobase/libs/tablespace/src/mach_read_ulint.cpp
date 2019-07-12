#include <innodb/tablespace/mach_read_ulint.h>

#include <innodb/machine/data.h>
#include <innodb/error/ut_error.h>

/** Read 1 to 4 bytes from a file page buffered in the buffer pool.
@param[in]	ptr	pointer where to read
@param[in]	type	MLOG_1BYTE, MLOG_2BYTES, or MLOG_4BYTES
@return value read */
uint32_t mach_read_ulint(const byte *ptr, mlog_id_t type) {
  switch (type) {
    case MLOG_1BYTE:
      return (mach_read_from_1(ptr));
    case MLOG_2BYTES:
      return (mach_read_from_2(ptr));
    case MLOG_4BYTES:
      return (mach_read_from_4(ptr));
    default:
      break;
  }

  ut_error;
}
