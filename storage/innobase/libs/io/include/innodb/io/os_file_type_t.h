#pragma once

#include <innodb/univ/univ.h>

/* File types for directory entry data type */

enum os_file_type_t {
  /** Get status failed. */
  OS_FILE_TYPE_FAILED,

  /** stat() failed, with ENAMETOOLONG */
  OS_FILE_TYPE_NAME_TOO_LONG,

  /** stat() failed with EACCESS */
  OS_FILE_PERMISSION_ERROR,

  /** File doesn't exist. */
  OS_FILE_TYPE_MISSING,

  /** File exists but type is unknown. */
  OS_FILE_TYPE_UNKNOWN,

  /** Ordinary file. */
  OS_FILE_TYPE_FILE,

  /** Directory. */
  OS_FILE_TYPE_DIR,

  /** Symbolic link. */
  OS_FILE_TYPE_LINK,

  /** Block device. */
  OS_FILE_TYPE_BLOCK
};
