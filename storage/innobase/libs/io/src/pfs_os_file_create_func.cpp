#include <innodb/io/pfs_os_file_create_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/register_pfs_file_open_begin.h>
#include <innodb/io/register_pfs_file_open_end.h>
#include <innodb/io/os_file_create_func.h>
#include <innodb/io/os_file_create_t.h>

/** NOTE! Please use the corresponding macro os_file_create(), not directly
this function!
A performance schema wrapper function for os_file_create().
Add instrumentation to monitor file creation/open.
@param[in]	key		Performance Schema Key
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	purpose		OS_FILE_AIO, if asynchronous, non-buffered I/O
                                is desired, OS_FILE_NORMAL, if any normal file;
                                NOTE that it also depends on type, os_aio_..
                                and srv_.. variables whether we really us
                                async I/O or unbuffered I/O: look in the
                                function source code for the exact rules
@param[in]	type		OS_DATA_FILE or OS_LOG_FILE
@param[in]	read_only	if true read only mode checks are enforced
@param[out]	success		true if succeeded
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return own: handle to the file, not defined if error, error number
can be retrieved with os_file_get_last_error */
pfs_os_file_t pfs_os_file_create_func(mysql_pfs_key_t key, const char *name,
                                      ulint create_mode, ulint purpose,
                                      ulint type, bool read_only, bool *success,
                                      const char *src_file, uint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;
  pfs_os_file_t file;

  /* register a file open or creation depending on "create_mode" */
  register_pfs_file_open_begin(
      &state, locker, key,
      create_mode == OS_FILE_CREATE ? PSI_FILE_CREATE : PSI_FILE_OPEN, name,
      src_file, src_line);

  file =
      os_file_create_func(name, create_mode, purpose, type, read_only, success);

  file.m_psi = NULL;
  register_pfs_file_open_end(locker, file, (*success == TRUE ? success : 0));

  return (file);
}

#endif
