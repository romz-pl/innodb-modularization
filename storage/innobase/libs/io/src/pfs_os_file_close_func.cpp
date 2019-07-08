#include <innodb/io/pfs_os_file_close_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/io/os_file_close_func.h>
#include <innodb/io/os_file_create_t.h>

/**
NOTE! Please use the corresponding macro os_file_close(), not directly
this function!
A performance schema instrumented wrapper function for os_file_close().
@param[in]	file		handle to a file
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return true if success */
bool pfs_os_file_close_func(pfs_os_file_t file, const char *src_file,
                            uint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;

  /* register the file close */
  register_pfs_file_io_begin(&state, locker, file, 0, PSI_FILE_CLOSE, src_file,
                             src_line);

  bool result = os_file_close_func(file.m_file);

  register_pfs_file_io_end(locker, 0);

  return (result);
}

#endif
