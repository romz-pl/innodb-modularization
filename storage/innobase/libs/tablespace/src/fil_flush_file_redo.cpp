#include <innodb/tablespace/fil_flush_file_redo.h>

#include <innodb/tablespace/fil_system.h>

/** Flush to disk the writes in file spaces of the given type
possibly cached by the OS. */
void fil_flush_file_redo() {
    fil_system->flush_file_redo();
}
