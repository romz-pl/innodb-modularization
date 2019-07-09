#include <innodb/io/os_file_close_no_error_handling.h>

#ifdef UNIV_HOTBACKUP

#ifndef _WIN32

    /** Closes a file handle.
    @param[in]	file		Handle to a file
    @return true if success */
    bool os_file_close_no_error_handling(os_file_t file) {
      return (close(file) != -1);
    }

#else

    /** Closes a file handle.
    @param[in]	file		Handle to close
    @return true if success */
    bool os_file_close_no_error_handling(os_file_t file) {
      return (CloseHandle(file) ? true : false);
    }

#endif

#endif /* UNIV_HOTBACKUP */
