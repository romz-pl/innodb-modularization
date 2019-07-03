#include <innodb/io/os_file_get_size.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/** Gets a file size.
@param[in]	filename	Full path to the filename to check
@return file size if OK, else set m_total_size to ~0 and m_alloc_size to
        errno */
os_file_size_t os_file_get_size(const char *filename) {
  struct stat s;
  os_file_size_t file_size;

  int ret = stat(filename, &s);

  if (ret == 0) {
    file_size.m_total_size = s.st_size;
    /* st_blocks is in 512 byte sized blocks */
    file_size.m_alloc_size = s.st_blocks * 512;
  } else {
    file_size.m_total_size = ~0;
    file_size.m_alloc_size = (os_offset_t)errno;
  }

  return (file_size);
}
