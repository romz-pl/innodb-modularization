#include <innodb/io/os_file_can_delete.h>

#include <innodb/io/Fil_path.h>
#include <innodb/logger/warn.h>

/** Check the file type and determine if it can be deleted.
@param[in]	name		Filename/Path to check
@return true if it's a file or a symlink and can be deleted */
bool os_file_can_delete(const char *name) {
  switch (Fil_path::get_file_type(name)) {
    case OS_FILE_TYPE_FILE:
    case OS_FILE_TYPE_LINK:
      return (true);

    case OS_FILE_TYPE_DIR:

      ib::warn(ER_IB_MSG_743) << "'" << name << "'"
                              << " is a directory, can't delete!";
      break;

    case OS_FILE_TYPE_BLOCK:

      ib::warn(ER_IB_MSG_744) << "'" << name << "'"
                              << " is a block device, can't delete!";
      break;

    case OS_FILE_TYPE_FAILED:

      ib::warn(ER_IB_MSG_745) << "'" << name << "'"
                              << " get file type failed, won't delete!";
      break;

    case OS_FILE_TYPE_UNKNOWN:

      ib::warn(ER_IB_MSG_746) << "'" << name << "'"
                              << " unknown file type, won't delete!";
      break;

    case OS_FILE_TYPE_NAME_TOO_LONG:

      ib::warn(ER_IB_MSG_747) << "'" << name << "'"
                              << " name too long, can't delete!";
      break;

    case OS_FILE_PERMISSION_ERROR:
      ib::warn(ER_IB_MSG_748) << "'" << name << "'"
                              << " permission error, can't delete!";
      break;

    case OS_FILE_TYPE_MISSING:
      break;
  }

  return (false);
}

