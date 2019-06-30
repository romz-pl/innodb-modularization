#include <innodb/string/innobase_basename.h>

#include "my_sys.h"

/** Strip dir name from a full path name and return only the file name
@param[in]	path_name	full path name
@return file name or "null" if no file name */
const char *innobase_basename(const char *path_name) {
  const char *name = base_name(path_name);

  return ((name) ? name : "null");
}
