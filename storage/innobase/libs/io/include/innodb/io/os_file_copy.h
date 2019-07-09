#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_copy_pfs.h>

#ifdef UNIV_PFS_IO
#define os_file_copy(src, src_offset, dest, dest_offset, size) \
  os_file_copy_pfs(src, src_offset, dest, dest_offset, size)
#else
#define os_file_copy(src, src_offset, dest, dest_offset, size) \
  os_file_copy_pfs(src.m_file, src_offset, dest.m_file, dest_offset, size)
#endif
