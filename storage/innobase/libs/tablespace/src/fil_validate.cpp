#include <innodb/tablespace/fil_validate.h>

#ifdef UNIV_DEBUG

#include <innodb/tablespace/fil_system.h>

/** Checks the consistency of the tablespace cache.
@return true if ok */
bool fil_validate() {
    return (fil_system->validate());
}

#endif /* UNIV_DEBUG */
