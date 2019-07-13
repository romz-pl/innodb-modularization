#include <innodb/tablespace/fil_system.h>

/** The tablespace memory cache. This variable is nullptr before the module is
initialized. */
Fil_system *fil_system = nullptr;
