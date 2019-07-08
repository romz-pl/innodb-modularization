#include <innodb/io/pfs_os_file_create_simple_no_error_handling_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/register_pfs_file_open_begin.h>
#include <innodb/io/register_pfs_file_open_end.h>
#include <innodb/io/os_file_create_simple_no_error_handling_func.h>
#include <innodb/io/os_file_create_t.h>

/** NOTE! Please use the corresponding macro
os_file_create_simple_no_error_handling(), not directly this function!
A performance schema instrumented wrapper function for
os_file_create_simple_no_error_handling(). Add instrumentation to
monitor file creation/open.
@param[in]	key		Performance Schema Key
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	access_type	OS_FILE_READ_ONLY, OS_FILE_READ_WRITE, or
                                OS_FILE_READ_ALLOW_DELETE; the last option is
                                used by a backup program reading the file
@param[in]	read_only	if true read only mode checks are enforced
@param[out]	success		true if succeeded
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return own: handle to the file, not defined if error, error number
can be retrieved with os_file_get_last_error */
pfs_os_file_t pfs_os_file_create_simple_no_error_handling_func(
    mysql_pfs_key_t key, const char *name, ulint create_mode, ulint access_type,
    bool read_only, bool *success, const char *src_file, uint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;
  pfs_os_file_t file;

  /* register a file open or creation depending on "create_mode" */
  register_pfs_file_open_begin(
      &state, locker, key,
      create_mode == OS_FILE_CREATE ? PSI_FILE_CREATE : PSI_FILE_OPEN, name,
      src_file, src_line);

  file = os_file_create_simple_no_error_handling_func(
      name, create_mode, access_type, read_only, success);

  file.m_psi = NULL;
  register_pfs_file_open_end(locker, file, (*success == TRUE ? success : 0));

  return (file);
}


#endif
