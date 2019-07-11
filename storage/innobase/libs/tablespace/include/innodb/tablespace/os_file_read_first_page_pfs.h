#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/tablespace/pfs_os_file_read_first_page_func.h>

#define os_file_read_first_page_pfs(type, file, buf, n) \
  pfs_os_file_read_first_page_func(type, file, buf, n, __FILE__, __LINE__)


#else /* UNIV_PFS_IO */

#include <innodb/tablespace/os_file_read_first_page_func.h>

#define os_file_read_first_page_pfs(type, file, buf, n) \
  os_file_read_first_page_func(type, file, buf, n)

#endif /* UNIV_PFS_IO */
