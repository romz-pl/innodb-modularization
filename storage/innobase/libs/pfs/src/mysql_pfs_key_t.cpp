#include <innodb/pfs/mysql_pfs_key_t.h>

#ifdef HAVE_PSI_INTERFACE

/** To keep count of number of PS keys defined. */
unsigned int mysql_pfs_key_t::s_count = 0;

#endif /* HAVE_PSI_INTERFACE */
