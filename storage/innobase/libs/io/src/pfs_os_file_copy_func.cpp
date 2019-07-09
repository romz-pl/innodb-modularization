#include <innodb/io/pfs_os_file_copy_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/io/os_file_copy_func.h>
#include <innodb/io/os_file_create_t.h>

#include "mysql/components/services/psi_file_bits.h"

/** Copy data from one file to another file. Data is read/written
at current file offset.
@param[in]	src		file handle to copy from
@param[in]	src_offset	offset to copy from
@param[in]	dest		file handle to copy to
@param[in]	dest_offset	offset to copy to
@param[in]	size		number of bytes to copy
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if successful */
dberr_t pfs_os_file_copy_func(pfs_os_file_t src, os_offset_t src_offset,
                              pfs_os_file_t dest, os_offset_t dest_offset,
                              uint size, const char *src_file, uint src_line) {
  dberr_t result;

  PSI_file_locker_state state_read;
  PSI_file_locker_state state_write;

  struct PSI_file_locker *locker_read = nullptr;
  struct PSI_file_locker *locker_write = nullptr;

  register_pfs_file_io_begin(&state_read, locker_read, src, size, PSI_FILE_READ,
                             src_file, src_line);

  register_pfs_file_io_begin(&state_write, locker_write, dest, size,
                             PSI_FILE_WRITE, src_file, src_line);

  result =
      os_file_copy_func(src.m_file, src_offset, dest.m_file, dest_offset, size);

  register_pfs_file_io_end(locker_write, size);
  register_pfs_file_io_end(locker_read, size);

  return (result);
}

#endif
