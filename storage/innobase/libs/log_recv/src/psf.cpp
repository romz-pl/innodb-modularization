#include <innodb/log_recv/psf.h>


#ifndef UNIV_HOTBACKUP
PSI_memory_key mem_log_recv_page_hash_key;
PSI_memory_key mem_log_recv_space_hash_key;
#endif /* !UNIV_HOTBACKUP */


#include <innodb/pfs/mysql_pfs_key_t.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_THREAD
mysql_pfs_key_t recv_writer_thread_key;
#endif /* UNIV_PFS_THREAD */
#endif /* !UNIV_HOTBACKUP */
