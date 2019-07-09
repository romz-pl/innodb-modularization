#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

/* Keys to register InnoDB I/O with performance schema */
extern mysql_pfs_key_t innodb_arch_file_key;

#endif
