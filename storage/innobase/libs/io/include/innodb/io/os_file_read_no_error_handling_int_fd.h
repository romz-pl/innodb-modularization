#pragma once

#include <innodb/univ/univ.h>

/* Following macros/functions are file I/O APIs that would be performance
schema instrumented if "UNIV_PFS_IO" is defined. They would point to
wrapper functions with performance schema instrumentation in such case.


If UNIV_PFS_IO is not defined, these I/O APIs point
to original un-instrumented file I/O APIs */

#ifdef UNIV_PFS_IO

#include <innodb/io/pfs_os_file_read_no_error_handling_int_fd_func.h>

#define os_file_read_no_error_handling_int_fd(type, file, buf, offset, n, o) \
  pfs_os_file_read_no_error_handling_int_fd_func(type, file, buf, offset, n, \
                                                 o, __FILE__, __LINE__)

#else

#include <innodb/io/os_file_read_no_error_handling_func.h>

#define os_file_read_no_error_handling_int_fd(type, file, buf, offset, n, o) \
  os_file_read_no_error_handling_func(type, file, buf, offset, n, o)

#endif
