#include <innodb/log_redo/pfs.h>

#ifdef UNIV_PFS_THREAD

#include <innodb/pfs/mysql_pfs_key_t.h>

/** PFS key for the log writer thread. */
mysql_pfs_key_t log_writer_thread_key;

/** PFS key for the log closer thread. */
mysql_pfs_key_t log_closer_thread_key;

/** PFS key for the log checkpointer thread. */
mysql_pfs_key_t log_checkpointer_thread_key;

/** PFS key for the log flusher thread. */
mysql_pfs_key_t log_flusher_thread_key;

/** PFS key for the log flush notifier thread. */
mysql_pfs_key_t log_flush_notifier_thread_key;

/** PFS key for the log write notifier thread. */
mysql_pfs_key_t log_write_notifier_thread_key;

mysql_pfs_key_t log_checkpointer_mutex_key;
mysql_pfs_key_t log_closer_mutex_key;
mysql_pfs_key_t log_writer_mutex_key;
mysql_pfs_key_t log_flusher_mutex_key;
mysql_pfs_key_t log_write_notifier_mutex_key;
mysql_pfs_key_t log_flush_notifier_mutex_key;
mysql_pfs_key_t log_cmdq_mutex_key;
mysql_pfs_key_t log_sn_lock_key;
mysql_pfs_key_t log_sys_arch_mutex_key;

#endif /* UNIV_PFS_THREAD */
