#include <innodb/io/os_file_create_subdirs_if_needed.h>

#include <innodb/logger/error.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/io/os_file_create_directory.h>
#include <innodb/io/os_file_status.h>
#include <innodb/io/os_file_get_parent_dir.h>

extern bool srv_read_only_mode;


/** Creates all missing subdirectories along the given path.
@param[in]	path		Path name
@return DB_SUCCESS if OK, otherwise error code. */
dberr_t os_file_create_subdirs_if_needed(const char *path) {
  if (srv_read_only_mode) {
    ib::error(ER_IB_MSG_753) << "read only mode set. Can't create "
                             << "subdirectories '" << path << "'";

    return (DB_READ_ONLY);
  }

  char *subdir = os_file_get_parent_dir(path);

  if (subdir == NULL) {
    /* subdir is root or cwd, nothing to do */
    return (DB_SUCCESS);
  }

  /* Test if subdir exists */
  os_file_type_t type;
  bool subdir_exists;
  bool success = os_file_status(subdir, &subdir_exists, &type);

  if (success && !subdir_exists) {
    /* Subdir does not exist, create it */
    dberr_t err = os_file_create_subdirs_if_needed(subdir);

    if (err != DB_SUCCESS) {
      ut_free(subdir);

      return (err);
    }

    success = os_file_create_directory(subdir, false);
  }

  ut_free(subdir);

  return (success ? DB_SUCCESS : DB_ERROR);
}

