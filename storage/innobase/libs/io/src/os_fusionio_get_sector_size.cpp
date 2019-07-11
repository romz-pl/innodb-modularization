#include <innodb/io/os_fusionio_get_sector_size.h>

#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)

#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/io/os_file_t.h>
#include <innodb/io/os_io_ptr_align.h>
#include <innodb/io/srv_unix_flush_t.h>
#include <innodb/logger/error.h>
#include <innodb/io/srv_unix_file_flush_method.h>

#include "my_io.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern char *srv_data_home;

/** Max disk sector size */
static const ulint MAX_SECTOR_SIZE = 4096;

/**
Try and get the FusionIO sector size. */
void os_fusionio_get_sector_size() {
  if (srv_unix_file_flush_method == SRV_UNIX_O_DIRECT ||
      srv_unix_file_flush_method == SRV_UNIX_O_DIRECT_NO_FSYNC) {
    ulint sector_size = UNIV_SECTOR_SIZE;
    char *path = srv_data_home;
    os_file_t check_file;
    byte *ptr;
    byte *block_ptr;
    char current_dir[3];
    char *dir_end;
    ulint dir_len;
    ulint check_path_len;
    char *check_file_name;
    ssize_t ret;

    /* If the srv_data_home is empty, set the path to
    current dir. */
    if (*path == 0) {
      current_dir[0] = FN_CURLIB;
      current_dir[1] = FN_LIBCHAR;
      current_dir[2] = 0;
      path = current_dir;
    }

    /* Get the path of data file */
    dir_end = strrchr(path, OS_PATH_SEPARATOR);
    dir_len = dir_end ? dir_end - path : strlen(path);

    /* allocate a new path and move the directory path to it. */
    check_path_len = dir_len + sizeof "/check_sector_size";
    check_file_name = static_cast<char *>(ut_zalloc_nokey(check_path_len));
    memcpy(check_file_name, path, dir_len);

    /* Construct a check file name. */
    strcat(check_file_name + dir_len, "/check_sector_size");

    /* Create a tmp file for checking sector size. */
    check_file = ::open(check_file_name,
                        O_CREAT | O_TRUNC | O_WRONLY | O_DIRECT, S_IRWXU);

    if (check_file == -1) {
      ib::error(ER_IB_MSG_830)
          << "Failed to create check sector file, errno:" << errno
          << " Please confirm O_DIRECT is"
          << " supported and remove the file " << check_file_name
          << " if it exists.";
      ut_free(check_file_name);
      errno = 0;
      return;
    }

    /* Try to write the file with different sector size
    alignment. */
    ptr = static_cast<byte *>(ut_zalloc_nokey(2 * MAX_SECTOR_SIZE));

    while (sector_size <= MAX_SECTOR_SIZE) {
      block_ptr = static_cast<byte *>(ut_align(ptr, sector_size));
      ret = pwrite(check_file, block_ptr, sector_size, 0);
      if (ret > 0 && (ulint)ret == sector_size) {
        break;
      }
      sector_size *= 2;
    }

    /* The sector size should <= MAX_SECTOR_SIZE. */
    ut_ad(sector_size <= MAX_SECTOR_SIZE);

    close(check_file);
    unlink(check_file_name);

    ut_free(check_file_name);
    ut_free(ptr);
    errno = 0;

    os_io_ptr_align = sector_size;
  }
}
#endif /* !NO_FALLOCATE && UNIV_LINUX */


