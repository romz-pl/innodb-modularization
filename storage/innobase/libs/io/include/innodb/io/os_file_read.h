#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_read_pfs.h>

#ifdef UNIV_PFS_IO
#define os_file_read(type, file, buf, offset, n) \
  os_file_read_pfs(type, file, buf, offset, n)
#else
#define os_file_read(type, file, buf, offset, n) \
  os_file_read_pfs(type, file.m_file, buf, offset, n)
#endif
