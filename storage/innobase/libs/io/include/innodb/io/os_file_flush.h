#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_flush.h>

#ifdef UNIV_PFS_IO
#define os_file_flush(file) os_file_flush_pfs(file)
#else
#define os_file_flush(file) os_file_flush_pfs(file.m_file)
#endif
