#include <innodb/io/os_file_set_nocache.h>

#include <innodb/logger/error.h>
#include <innodb/logger/warn.h>

#include <unistd.h>
#include <fcntl.h>

/** Tries to disable OS caching on an opened file descriptor.
@param[in]	fd		file descriptor to alter
@param[in]	file_name	file name, used in the diagnostic message
@param[in]	operation_name	"open" or "create"; used in the diagnostic
                                message */
void os_file_set_nocache(int fd MY_ATTRIBUTE((unused)),
                         const char *file_name MY_ATTRIBUTE((unused)),
                         const char *operation_name MY_ATTRIBUTE((unused))) {
/* some versions of Solaris may not have DIRECTIO_ON */
#if defined(UNIV_SOLARIS) && defined(DIRECTIO_ON)
  if (directio(fd, DIRECTIO_ON) == -1) {
    int errno_save = errno;

    ib::error(ER_IB_MSG_823)
        << "Failed to set DIRECTIO_ON on file " << file_name << "; "
        << operation_name << ": " << strerror(errno_save)
        << ","
           " continuing anyway.";
  }
#elif defined(O_DIRECT)
  if (fcntl(fd, F_SETFL, O_DIRECT) == -1) {
    int errno_save = errno;
    static bool warning_message_printed = false;
    if (errno_save == EINVAL) {
      if (!warning_message_printed) {
        warning_message_printed = true;
#ifdef UNIV_LINUX
        ib::warn(ER_IB_MSG_824)
            << "Failed to set O_DIRECT on file" << file_name << "; "
            << operation_name << ": " << strerror(errno_save)
            << ", "
               "continuing anyway. O_DIRECT is "
               "known to result in 'Invalid argument' "
               "on Linux on tmpfs, "
               "see MySQL Bug#26662.";
#else  /* UNIV_LINUX */
        goto short_warning;
#endif /* UNIV_LINUX */
      }
    } else {
#ifndef UNIV_LINUX
    short_warning:
#endif
      ib::warn(ER_IB_MSG_825) << "Failed to set O_DIRECT on file " << file_name
                              << "; " << operation_name << " : "
                              << strerror(errno_save) << ", continuing anyway.";
    }
  }
#endif /* defined(UNIV_SOLARIS) && defined(DIRECTIO_ON) */
}
