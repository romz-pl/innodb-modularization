#include <innodb/sync_latch/sync_basename.h>

#ifndef UNIV_LIBRARY
#ifdef UNIV_PFS_MUTEX
#ifndef UNIV_HOTBACKUP

/** Print the filename "basename" e.g., p = "/a/b/c/d/e.cc" -> p = "e.cc"
@param[in]	filename	Name from where to extract the basename
@return the basename */
const char *sync_basename(const char *filename) {
  const char *ptr = filename + strlen(filename) - 1;

  while (ptr > filename && *ptr != '/' && *ptr != '\\') {
    --ptr;
  }

  ++ptr;

  return (ptr);
}


#endif
#endif
#endif
