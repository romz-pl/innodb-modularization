#include <innodb/io/os_file_get_status_posix.h>

#include <innodb/io/os_file_handle_error_no_exit.h>
#include <innodb/io/os_innodb_umask.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/** This function returns information about the specified file
@param[in]	path		pathname of the file
@param[out]	stat_info	information of a file in a directory
@param[in,out]	statinfo	information of a file in a directory
@param[in]	check_rw_perm	for testing whether the file can be opened
                                in RW mode
@param[in]	read_only	if true read only mode checks are enforced
@return DB_SUCCESS if all OK */
dberr_t os_file_get_status_posix(const char *path,
                                        os_file_stat_t *stat_info,
                                        struct stat *statinfo,
                                        bool check_rw_perm, bool read_only) {
  int ret = stat(path, statinfo);

  if (ret && (errno == ENOENT || errno == ENOTDIR)) {
    /* file does not exist */

    return (DB_NOT_FOUND);

  } else if (ret) {
    /* file exists, but stat call failed */

    os_file_handle_error_no_exit(path, "stat", false);

    return (DB_FAIL);
  }

  switch (statinfo->st_mode & S_IFMT) {
    case S_IFDIR:
      stat_info->type = OS_FILE_TYPE_DIR;
      break;
    case S_IFLNK:
      stat_info->type = OS_FILE_TYPE_LINK;
      break;
    case S_IFBLK:
      /* Handle block device as regular file. */
    case S_IFCHR:
      /* Handle character device as regular file. */
    case S_IFREG:
      stat_info->type = OS_FILE_TYPE_FILE;
      break;
    default:
      stat_info->type = OS_FILE_TYPE_UNKNOWN;
  }

  stat_info->size = statinfo->st_size;
  stat_info->block_size = statinfo->st_blksize;
  stat_info->alloc_size = statinfo->st_blocks * 512;

  if (check_rw_perm && (stat_info->type == OS_FILE_TYPE_FILE ||
                        stat_info->type == OS_FILE_TYPE_BLOCK)) {
    int access = !read_only ? O_RDWR : O_RDONLY;
    int fh = ::open(path, access, os_innodb_umask);

    if (fh == -1) {
      stat_info->rw_perm = false;
    } else {
      stat_info->rw_perm = true;
      close(fh);
    }
  }

  return (DB_SUCCESS);
}

