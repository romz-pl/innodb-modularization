#pragma once

#include <innodb/univ/univ.h>


#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)

#include <innodb/io/pfs_os_file_t.h>

bool fil_fusionio_enable_atomic_write(pfs_os_file_t file);


#endif /* !NO_FALLOCATE && UNIV_LINUX */
