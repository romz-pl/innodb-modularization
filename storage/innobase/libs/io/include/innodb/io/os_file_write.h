#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_write_pfs.h>

#ifdef UNIV_PFS_IO
#define os_file_write(type, name, file, buf, offset, n) \
  os_file_write_pfs(type, name, file, buf, offset, n)
#else
#define os_file_write(type, name, file, buf, offset, n) \
  os_file_write_pfs(type, name, file.m_file, buf, offset, n)
#endif
