#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_THREAD

#include <innodb/pfs/mysql_pfs_key_t.h>

extern mysql_pfs_key_t page_archiver_thread_key;
extern mysql_pfs_key_t log_archiver_thread_key;

#endif
