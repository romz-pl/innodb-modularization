#pragma once

#include <innodb/univ/univ.h>


#ifdef _WIN32

/** File handle */
#define os_file_t HANDLE

#else

/** File handle */
typedef int os_file_t;

#endif

const os_file_t OS_FILE_CLOSED = os_file_t(~0);
