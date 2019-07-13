#include <innodb/tablespace/fil_free_scanned_files.h>

#include <innodb/tablespace/fil_system.h>

/** Free the data structures required for recovery. */
void fil_free_scanned_files() {
    fil_system->free_scanned_files();
}
