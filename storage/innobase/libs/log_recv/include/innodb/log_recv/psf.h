#pragma once

#include <innodb/univ/univ.h>

#include "mysql/components/services/psi_memory_bits.h"

#ifndef UNIV_HOTBACKUP
extern PSI_memory_key mem_log_recv_page_hash_key;
extern PSI_memory_key mem_log_recv_space_hash_key;
#endif /* !UNIV_HOTBACKUP */


#include <innodb/pfs/mysql_pfs_key_t.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_THREAD
extern mysql_pfs_key_t recv_writer_thread_key;
#endif /* UNIV_PFS_THREAD */
#endif /* !UNIV_HOTBACKUP */
