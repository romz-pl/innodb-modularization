#include <innodb/io/os_file_create_tmpfile.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/logger/error.h>

#include <unistd.h>

int innobase_mysql_tmpfile(const char *path);

/** Creates a temporary file.  This function is like tmpfile(3), but
the temporary file is created in the given parameter path. If the path
is NULL then it will create the file in the MySQL server configuration
parameter (--tmpdir).
@param[in]	path	location for creating temporary file
@return temporary file handle, or NULL on error */
FILE *os_file_create_tmpfile(const char *path) {
  FILE *file = NULL;
  int fd = innobase_mysql_tmpfile(path);

  if (fd >= 0) {
    file = fdopen(fd, "w+b");
  }

  if (file == NULL) {
    ib::error(ER_IB_MSG_751)
        << "Unable to create temporary file; errno: " << errno;

    if (fd >= 0) {
      close(fd);
    }
  }

  return (file);
}

#endif /* !UNIV_HOTBACKUP */
