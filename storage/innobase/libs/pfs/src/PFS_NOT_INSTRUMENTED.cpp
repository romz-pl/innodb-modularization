#include <innodb/pfs/PFS_NOT_INSTRUMENTED.h>


/* There are mutexes/rwlocks that we want to exclude from instrumentation
even if their corresponding performance schema define is set. And this
PFS_NOT_INSTRUMENTED is used as the key value to identify those objects that
would be excluded from instrumentation.*/
mysql_pfs_key_t PFS_NOT_INSTRUMENTED(UINT32_UNDEFINED);
