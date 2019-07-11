#include <innodb/tablespace/pfs_os_file_read_first_page_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/IORequest.h>
#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/tablespace/os_file_read_first_page_func.h>


/** NOTE! Please use the corresponding macro os_file_read_first_page(),
not directly this function!
This is the performance schema instrumented wrapper function for
os_file_read() which requests a synchronous read operation.
@param[in, out]	type		IO request context
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	n		number of bytes to read
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if request was successful */
dberr_t pfs_os_file_read_first_page_func(IORequest &type, pfs_os_file_t file,
                                         void *buf, ulint n,
                                         const char *src_file, uint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;

  ut_ad(type.validate());

  register_pfs_file_io_begin(&state, locker, file, n, PSI_FILE_READ, src_file,
                             src_line);

  dberr_t result;

  result = os_file_read_first_page_func(type, file.m_file, buf, n);

  register_pfs_file_io_end(locker, n);

  return (result);
}


#endif /* UNIV_PFS_IO */
