#include <innodb/io/os_file_scan_directory.h>

#include <innodb/io/os_file_handle_error_no_exit.h>

#include <sys/types.h>
#include <dirent.h>

/** This function scans the contents of a directory and invokes the callback
for each entry.
@param[in]	path		directory name as null-terminated string
@param[in]	scan_cbk	use callback to be called for each entry
@param[in]	is_drop		attempt to drop the directory after scan
@return true if call succeeds, false on error */
bool os_file_scan_directory(const char *path, os_dir_cbk_t scan_cbk,
                            bool is_drop) {
  DIR *directory;
  dirent *entry;

  directory = opendir(path);

  if (directory == nullptr) {
    os_file_handle_error_no_exit(path, "opendir", false);
    return (false);
  }

  entry = readdir(directory);

  while (entry != nullptr) {
    scan_cbk(path, entry->d_name);
    entry = readdir(directory);
  }

  closedir(directory);

  if (is_drop) {
    int err;
    err = rmdir(path);

    if (err != 0) {
      os_file_handle_error_no_exit(path, "rmdir", false);
      return (false);
    }
  }

  return (true);
}

