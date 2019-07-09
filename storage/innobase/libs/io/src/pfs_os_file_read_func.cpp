#include <innodb/io/pfs_os_file_read_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/io/os_file_read_func.h>
#include <innodb/assert/assert.h>

#include "mysql/components/services/psi_file_bits.h"

/** NOTE! Please use the corresponding macro os_file_read(), not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_read() which requests a synchronous read operation.
@param[in, out]	type		IO request context
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if request was successful */
dberr_t pfs_os_file_read_func(IORequest &type, pfs_os_file_t file, void *buf,
                              os_offset_t offset, ulint n, const char *src_file,
                              uint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;

  ut_ad(type.validate());

  register_pfs_file_io_begin(&state, locker, file, n, PSI_FILE_READ, src_file,
                             src_line);

  dberr_t result;

  result = os_file_read_func(type, file.m_file, buf, offset, n);

  register_pfs_file_io_end(locker, n);

  return (result);
}

#endif
