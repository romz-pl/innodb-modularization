#include <innodb/io/os_file_get_status.h>

#include <innodb/io/os_file_get_status_posix.h>

/** This function returns information about the specified file
@param[in]	path		pathname of the file
@param[out]	stat_info	information of a file in a directory
@param[in]	check_rw_perm	for testing whether the file can be opened
                                in RW mode
@param[in]	read_only	true if file is opened in read-only mode
@return DB_SUCCESS if all OK */
dberr_t os_file_get_status(const char *path, os_file_stat_t *stat_info,
                           bool check_rw_perm, bool read_only) {
  dberr_t ret;

#ifdef _WIN32
  struct _stat64 info;

  ret = os_file_get_status_win32(path, stat_info, &info, check_rw_perm,
                                 read_only);

#else
  struct stat info;

  ret = os_file_get_status_posix(path, stat_info, &info, check_rw_perm,
                                 read_only);

#endif /* _WIN32 */

  if (ret == DB_SUCCESS) {
    stat_info->ctime = info.st_ctime;
    stat_info->atime = info.st_atime;
    stat_info->mtime = info.st_mtime;
    stat_info->size = info.st_size;
  }

  return (ret);
}
