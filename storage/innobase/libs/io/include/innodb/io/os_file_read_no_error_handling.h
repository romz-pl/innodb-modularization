#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_read_no_error_handling_pfs.h>

#ifdef UNIV_PFS_IO
#define os_file_read_no_error_handling(type, file, buf, offset, n, o) \
  os_file_read_no_error_handling_pfs(type, file, buf, offset, n, o)
#else
#define os_file_read_no_error_handling(type, file, buf, offset, n, o) \
  os_file_read_no_error_handling_pfs(type, file.m_file, buf, offset, n, o)
#endif
