#pragma once

#include <innodb/univ/univ.h>

/** Options for os_file_create_func */
enum os_file_create_t {
  OS_FILE_OPEN = 51,   /*!< to open an existing file (if
                       doesn't exist, error) */
  OS_FILE_CREATE,      /*!< to create new file (if
                       exists, error) */
  OS_FILE_OPEN_RAW,    /*!< to open a raw device or disk
                       partition */
  OS_FILE_CREATE_PATH, /*!< to create the directories */
  OS_FILE_OPEN_RETRY,  /*!< open with retry */

  /** Flags that can be combined with the above values. Please ensure
  that the above values stay below 128. */

  OS_FILE_ON_ERROR_NO_EXIT = 128, /*!< do not exit on unknown errors */
  OS_FILE_ON_ERROR_SILENT = 256   /*!< don't print diagnostic messages to
                                  the log unless it is a fatal error,
                                  this flag is only used if
                                  ON_ERROR_NO_EXIT is set */
};
