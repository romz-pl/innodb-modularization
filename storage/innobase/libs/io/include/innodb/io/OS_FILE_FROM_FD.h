#pragma once

#include <innodb/univ/univ.h>

#ifdef _WIN32

/** Convert a C file descriptor to a native file handle
@param fd file descriptor
@return native file handle */
#define OS_FILE_FROM_FD(fd) (HANDLE) _get_osfhandle(fd)

#else

/** Convert a C file descriptor to a native file handle
@param fd file descriptor
@return native file handle */
#define OS_FILE_FROM_FD(fd) fd

#endif
