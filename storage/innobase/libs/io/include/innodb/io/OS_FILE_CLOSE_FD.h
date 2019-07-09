#pragma once

#include <innodb/univ/univ.h>

#ifdef _WIN32

/** Closes the file associated with C file descriptor fd
@param[in]	fd	C file descriptor
@return 0 if success */
#define OS_FILE_CLOSE_FD(fd) _close(fd)

#else

#include <innodb/io/os_file_close.h>

/** Closes the file associated with C file descriptor fd
@param[in]	fd	C file descriptor
@return 0 if success */
#define OS_FILE_CLOSE_FD(fd) (os_file_close(fd) ? 0 : -1)


#endif
