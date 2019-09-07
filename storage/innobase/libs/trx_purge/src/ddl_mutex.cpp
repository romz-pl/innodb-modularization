#include <innodb/trx_purge/ddl_mutex.h>

namespace undo {

/** Mutext for serializing undo tablespace related DDL.  These have to do with
creating and dropping undo tablespaces. */
ib_mutex_t ddl_mutex;

}
