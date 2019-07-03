#include <innodb/io/os_file_status_posix.h>

#include <innodb/io/os_file_handle_error_no_exit.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


/** Check the existence and type of the given file.
@param[in]	path		path name of file
@param[out]	exists		true if the file exists
@param[out]	type		Type of the file, if it exists
@return true if call succeeded */
bool os_file_status_posix(const char *path, bool *exists,
                                 os_file_type_t *type) {
  struct stat statinfo;

  int ret = stat(path, &statinfo);

  *exists = !ret;

  if (!ret) {
    /* file exists, everything OK */

  } else if (errno == ENOENT || errno == ENOTDIR) {
    if (exists != nullptr) {
      *exists = false;
    }

    /* file does not exist */
    *type = OS_FILE_TYPE_MISSING;
    return (true);

  } else if (errno == ENAMETOOLONG) {
    *type = OS_FILE_TYPE_NAME_TOO_LONG;
    return (false);
  } else if (errno == EACCES) {
    *type = OS_FILE_PERMISSION_ERROR;
    return (false);
  } else {
    *type = OS_FILE_TYPE_FAILED;

    /* file exists, but stat call failed */
    os_file_handle_error_no_exit(path, "stat", false);
    return (false);
  }

  if (exists != nullptr) {
    *exists = true;
  }

  if (S_ISDIR(statinfo.st_mode)) {
    *type = OS_FILE_TYPE_DIR;

  } else if (S_ISLNK(statinfo.st_mode)) {
    *type = OS_FILE_TYPE_LINK;

  } else if (S_ISREG(statinfo.st_mode)) {
    *type = OS_FILE_TYPE_FILE;

  } else {
    *type = OS_FILE_TYPE_UNKNOWN;
  }

  return (true);
}

