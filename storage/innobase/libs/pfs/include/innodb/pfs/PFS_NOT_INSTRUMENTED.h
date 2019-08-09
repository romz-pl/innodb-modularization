#pragma once

#include <innodb/univ/univ.h>

#ifdef HAVE_PSI_INTERFACE

#include <innodb/pfs/mysql_pfs_key_t.h>
/* There are mutexes/rwlocks that we want to exclude from instrumentation
even if their corresponding performance schema define is set. And this
PFS_NOT_INSTRUMENTED is used as the key value to identify those objects that
would be excluded from instrumentation.*/
extern mysql_pfs_key_t PFS_NOT_INSTRUMENTED;

#include <innodb/pfs/mysql_pfs_key_t.h>
#endif /* HAVE_PFS_INTERFACE */
