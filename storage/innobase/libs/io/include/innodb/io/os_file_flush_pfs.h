#pragma once

#include <innodb/univ/univ.h>

/* Following macros/functions are file I/O APIs that would be performance
schema instrumented if "UNIV_PFS_IO" is defined. They would point to
wrapper functions with performance schema instrumentation in such case.


If UNIV_PFS_IO is not defined, these I/O APIs point
to original un-instrumented file I/O APIs */

#ifdef UNIV_PFS_IO

#include <innodb/io/pfs_os_file_flush_func.h>

#define os_file_flush_pfs(file) pfs_os_file_flush_func(file, __FILE__, __LINE__)

#else

#include <innodb/io/os_file_flush_func.h>

#define os_file_flush_pfs(file) os_file_flush_func(file)

#endif
