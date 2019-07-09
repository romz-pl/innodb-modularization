#pragma once

#include <innodb/univ/univ.h>

#ifdef _WIN32

/** Associates a C file descriptor with an existing native file handle
@param[in]	file	native file handle
@return C file descriptor */
#define OS_FD_FROM_FILE(file) _open_osfhandle((intptr_t)file, _O_RDONLY)

#else

/** C file descriptor from an existing native file handle
@param[in]	file	native file handle
@return C file descriptor */
#define OS_FD_FROM_FILE(file) file

#endif
