#include <innodb/io/pfs_os_file_delete_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/register_pfs_file_close_begin.h>
#include <innodb/io/register_pfs_file_close_end.h>
#include <innodb/io/os_file_delete_func.h>

#include "mysql/components/services/psi_file_bits.h"

/** NOTE! Please use the corresponding macro os_file_delete(), not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_delete()
@param[in]	key		Performance Schema Key
@param[in]	name		old file path as a null-terminated string
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return true if success */
bool pfs_os_file_delete_func(mysql_pfs_key_t key, const char *name,
                             const char *src_file, uint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;

  register_pfs_file_close_begin(&state, locker, key, PSI_FILE_DELETE, name,
                                src_file, src_line);

  bool result = os_file_delete_func(name);

  register_pfs_file_close_end(locker, 0);

  return (result);
}


#endif
