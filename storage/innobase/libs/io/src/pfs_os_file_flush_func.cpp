#include <innodb/io/pfs_os_file_flush_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/io/os_file_flush_func.h>

#include "mysql/components/services/psi_file_bits.h"

/** NOTE! Please use the corresponding macro os_file_flush(),
 not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_flush() which flushes the write buffers of a given file to the disk.
Flushes the write buffers of a given file to the disk.
@param[in]	file		Open file handle
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return true if success */
bool pfs_os_file_flush_func(pfs_os_file_t file, const char *src_file,
                            uint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;

  register_pfs_file_io_begin(&state, locker, file, 0, PSI_FILE_SYNC, src_file,
                             src_line);

  bool result = os_file_flush_func(file.m_file);

  register_pfs_file_io_end(locker, 0);

  return (result);
}

#endif
