#include <innodb/io/os_file_lock.h>

#include <innodb/logger/error.h>
#include <innodb/logger/info.h>

#undef USE_FILE_LOCK
#define USE_FILE_LOCK


#if defined(UNIV_HOTBACKUP) || defined(_WIN32)
/* InnoDB Hot Backup does not lock the data files.
 * On Windows, mandatory locking is used.
 */
#undef USE_FILE_LOCK
#endif /* UNIV_HOTBACKUP || _WIN32 */


#ifdef USE_FILE_LOCK

/** Obtain an exclusive lock on a file.
@param[in]	fd		file descriptor
@param[in]	name		file name
@return 0 on success */
int os_file_lock(int fd, const char *name) {
  struct flock lk;

  lk.l_type = F_WRLCK;
  lk.l_whence = SEEK_SET;
  lk.l_start = lk.l_len = 0;

  if (fcntl(fd, F_SETLK, &lk) == -1) {
    ib::error(ER_IB_MSG_749)
        << "Unable to lock " << name << " error: " << errno;

    if (errno == EAGAIN || errno == EACCES) {
      ib::info(ER_IB_MSG_750) << "Check that you do not already have"
                                 " another mysqld process using the"
                                 " same InnoDB data or log files.";
    }

    return (-1);
  }

  return (0);
}

#endif /* USE_FILE_LOCK */
