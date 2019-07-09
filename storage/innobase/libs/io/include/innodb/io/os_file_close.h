#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_close_pfs.h>

#ifdef UNIV_PFS_IO
#define os_file_close(file) os_file_close_pfs(file)
#else
#define os_file_close(file) os_file_close_pfs((file).m_file)
#endif
