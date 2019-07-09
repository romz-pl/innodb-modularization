#include <innodb/io/pfs_os_file_rename_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/register_pfs_file_rename_begin.h>
#include <innodb/io/register_pfs_file_rename_end.h>
#include <innodb/io/os_file_rename_func.h>

#include "mysql/components/services/psi_file_bits.h"

/** NOTE! Please use the corresponding macro os_file_rename(), not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_rename()
@param[in]	key		Performance Schema Key
@param[in]	oldpath		old file path as a null-terminated string
@param[in]	newpath		new file path
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return true if success */
bool pfs_os_file_rename_func(mysql_pfs_key_t key, const char *oldpath,
                             const char *newpath, const char *src_file,
                             uint src_line)

{
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;

  register_pfs_file_rename_begin(&state, locker, key, PSI_FILE_RENAME, newpath,
                                 src_file, src_line);

  bool result = os_file_rename_func(oldpath, newpath);

  register_pfs_file_rename_end(locker, oldpath, newpath, !result);

  return (result);
}


#endif
