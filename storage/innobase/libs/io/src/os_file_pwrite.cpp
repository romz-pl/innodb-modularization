#include <innodb/io/os_file_pwrite.h>

#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_io.h>
#include <innodb/io/os_n_file_writes.h>
#include <innodb/io/os_n_pending_writes.h>
#include <innodb/monitor/MONITOR_ATOMIC_DEC.h>
#include <innodb/monitor/MONITOR_ATOMIC_INC.h>

/** Does a synchronous write operation in Posix.
@param[in]	type		IO context
@param[in]	file		handle to an open file
@param[out]	buf		buffer from which to write
@param[in]	n		number of bytes to read, starting from offset
@param[in]	offset		file offset from the start where to read
@param[out]	err		DB_SUCCESS or error code
@return number of bytes written, -1 if error */
MY_ATTRIBUTE((warn_unused_result)) ssize_t
    os_file_pwrite(IORequest &type, os_file_t file, const byte *buf, ulint n,
                   os_offset_t offset, dberr_t *err) {
#ifdef UNIV_HOTBACKUP
  static meb::Mutex meb_mutex;
#endif /* UNIV_HOTBACKUP */

  ut_ad(type.validate());

#ifdef UNIV_HOTBACKUP
  meb_mutex.lock();
#endif /* UNIV_HOTBACKUP */
  ++os_n_file_writes;
#ifdef UNIV_HOTBACKUP
  meb_mutex.unlock();
#endif /* UNIV_HOTBACKUP */

  (void)os_atomic_increment_ulint(&os_n_pending_writes, 1);
  MONITOR_ATOMIC_INC(MONITOR_OS_PENDING_WRITES);

  ssize_t n_bytes = os_file_io(type, file, (void *)buf, n, offset, err);

  (void)os_atomic_decrement_ulint(&os_n_pending_writes, 1);
  MONITOR_ATOMIC_DEC(MONITOR_OS_PENDING_WRITES);

  return (n_bytes);
}
