#include <innodb/io/os_file_pread.h>

#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_io.h>
#include <innodb/io/os_n_file_reads.h>
#include <innodb/io/os_n_pending_reads.h>
#include <innodb/monitor/MONITOR_ATOMIC_DEC.h>
#include <innodb/monitor/MONITOR_ATOMIC_INC.h>

/** Does a synchronous read operation in Posix.
@param[in]	type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@param[out]	err		DB_SUCCESS or error code
@return number of bytes read, -1 if error */
MY_ATTRIBUTE((warn_unused_result)) ssize_t
    os_file_pread(IORequest &type, os_file_t file, void *buf, ulint n,
                  os_offset_t offset, dberr_t *err) {
#ifdef UNIV_HOTBACKUP
  static meb::Mutex meb_mutex;

  meb_mutex.lock();
#endif /* UNIV_HOTBACKUP */
  ++os_n_file_reads;
#ifdef UNIV_HOTBACKUP
  meb_mutex.unlock();
#endif /* UNIV_HOTBACKUP */

  (void)os_atomic_increment_ulint(&os_n_pending_reads, 1);
  MONITOR_ATOMIC_INC(MONITOR_OS_PENDING_READS);

  ssize_t n_bytes = os_file_io(type, file, buf, n, offset, err);

  (void)os_atomic_decrement_ulint(&os_n_pending_reads, 1);
  MONITOR_ATOMIC_DEC(MONITOR_OS_PENDING_READS);

  return (n_bytes);
}
