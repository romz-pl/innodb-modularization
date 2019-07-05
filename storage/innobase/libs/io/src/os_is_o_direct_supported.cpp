#include <innodb/io/os_is_o_direct_supported.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/io/os_file_t.h>

#include "my_io.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern char *srv_data_home;

/** Determine if O_DIRECT is supported
@retval	true	if O_DIRECT is supported.
@retval	false	if O_DIRECT is not supported. */
bool os_is_o_direct_supported() {
#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)
  char *path = srv_data_home;
  char *file_name;
  os_file_t file_handle;
  ulint dir_len;
  ulint path_len;
  bool add_os_path_separator = false;

  /* If the srv_data_home is empty, set the path to current dir. */
  char current_dir[3];
  if (*path == 0) {
    current_dir[0] = FN_CURLIB;
    current_dir[1] = FN_LIBCHAR;
    current_dir[2] = 0;
    path = current_dir;
  }

  /* Get the path length. */
  if (path[strlen(path) - 1] == OS_PATH_SEPARATOR) {
    /* path is ended with OS_PATH_SEPARATOR */
    dir_len = strlen(path);
  } else {
    /* path is not ended with OS_PATH_SEPARATOR */
    dir_len = strlen(path) + 1;
    add_os_path_separator = true;
  }

  /* Allocate a new path and move the directory path to it. */
  path_len = dir_len + sizeof "o_direct_test";
  file_name = static_cast<char *>(ut_zalloc_nokey(path_len));
  if (add_os_path_separator == true) {
    memcpy(file_name, path, dir_len - 1);
    file_name[dir_len - 1] = OS_PATH_SEPARATOR;
  } else {
    memcpy(file_name, path, dir_len);
  }

  /* Construct a temp file name. */
  strcat(file_name + dir_len, "o_direct_test");

  /* Try to create a temp file with O_DIRECT flag. */
  file_handle =
      ::open(file_name, O_CREAT | O_TRUNC | O_WRONLY | O_DIRECT, S_IRWXU);

  /* If Failed */
  if (file_handle == -1) {
    ut_free(file_name);
    return (false);
  }

  ::close(file_handle);
  unlink(file_name);
  ut_free(file_name);

  return (true);
#else
  return (false);
#endif /* !NO_FALLOCATE && UNIV_LINUX */
}
