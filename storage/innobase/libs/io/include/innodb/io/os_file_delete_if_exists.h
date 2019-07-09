#pragma once

#include <innodb/univ/univ.h>

/* Following macros/functions are file I/O APIs that would be performance
schema instrumented if "UNIV_PFS_IO" is defined. They would point to
wrapper functions with performance schema instrumentation in such case.


If UNIV_PFS_IO is not defined, these I/O APIs point
to original un-instrumented file I/O APIs */

#ifdef UNIV_PFS_IO

#include <innodb/io/pfs_os_file_delete_if_exists_func.h>

#define os_file_delete_if_exists(key, name, exist) \
  pfs_os_file_delete_if_exists_func(key, name, exist, __FILE__, __LINE__)

#else

#include <innodb/io/os_file_delete_if_exists_func.h>

#define os_file_delete_if_exists(key, name, exist) \
  os_file_delete_if_exists_func(name, exist)

#endif
