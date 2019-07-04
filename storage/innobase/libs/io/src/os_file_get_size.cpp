#include <innodb/io/os_file_get_size.h>

#include <innodb/io/pfs_os_file_t.h>

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

/** Gets a file size.
@param[in]	file		handle to an open file
@return file size, or (os_offset_t) -1 on failure */
os_offset_t os_file_get_size(pfs_os_file_t file) {
  /* Store current position */
  os_offset_t pos = lseek(file.m_file, 0, SEEK_CUR);
  os_offset_t file_size = lseek(file.m_file, 0, SEEK_END);
  /* Restore current position as the function should not change it */
  lseek(file.m_file, pos, SEEK_SET);
  return (file_size);
}
