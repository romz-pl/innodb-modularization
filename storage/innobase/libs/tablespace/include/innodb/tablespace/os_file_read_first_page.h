#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/os_file_read_first_page_pfs.h>

#ifdef UNIV_PFS_IO

#define os_file_read_first_page(type, file, buf, n) \
  os_file_read_first_page_pfs(type, file, buf, n)


#else

#define os_file_read_first_page(type, file, buf, n) \
  os_file_read_first_page_pfs(type, file.m_file, buf, n)

#endif
