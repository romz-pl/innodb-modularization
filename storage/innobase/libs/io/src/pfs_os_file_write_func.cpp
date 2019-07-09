#include <innodb/io/pfs_os_file_write_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/os_file_write_func.h>
#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/OS_FILE_FROM_FD.h>

#include "mysql/components/services/psi_file_bits.h"

/** NOTE! Please use the corresponding macro os_file_write(), not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_write() which requests a synchronous write operation.
@param[in, out]	type		IO request context
@param[in]	name		Name of the file or path as NUL terminated
                                string
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if request was successful */
dberr_t pfs_os_file_write_func(IORequest &type, const char *name,
                               pfs_os_file_t file, const void *buf,
                               os_offset_t offset, ulint n,
                               const char *src_file, uint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;

  register_pfs_file_io_begin(&state, locker, file, n, PSI_FILE_WRITE, src_file,
                             src_line);

  dberr_t result;

  result = os_file_write_func(type, name, file.m_file, buf, offset, n);

  register_pfs_file_io_end(locker, n);

  return (result);
}


#endif
