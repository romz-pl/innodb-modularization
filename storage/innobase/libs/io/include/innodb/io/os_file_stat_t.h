#pragma once

#include <innodb/univ/univ.h>
#include <innodb/io/os_file_type_t.h>
#include <innodb/io/os_offset_t.h>

#include "my_io.h"

/* Maximum path string length in bytes when referring to tables with in the
'./databasename/tablename.ibd' path format; we can allocate at least 2 buffers
of this size from the thread stack; that is why this should not be made much
bigger than 4000 bytes.  The maximum path length used by any storage engine
in the server must be at least this big. */
#define OS_FILE_MAX_PATH 4000
#if (FN_REFLEN_SE < OS_FILE_MAX_PATH)
#error "(FN_REFLEN_SE < OS_FILE_MAX_PATH)"
#endif

/** Struct used in fetching information of a file in a directory */
struct os_file_stat_t {
  char name[OS_FILE_MAX_PATH]; /*!< path to a file */
  os_file_type_t type;         /*!< file type */
  os_offset_t size;            /*!< file size in bytes */
  os_offset_t alloc_size;      /*!< Allocated size for
                               sparse files in bytes */
  uint32_t block_size;         /*!< Block size to use for IO
                               in bytes*/
  time_t ctime;                /*!< creation time */
  time_t mtime;                /*!< modification time */
  time_t atime;                /*!< access time */
  bool rw_perm;                /*!< true if can be opened
                               in read-write mode. Only valid
                               if type == OS_FILE_TYPE_FILE */
};
