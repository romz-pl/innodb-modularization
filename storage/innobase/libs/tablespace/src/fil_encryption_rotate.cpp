#include <innodb/tablespace/fil_encryption_rotate.h>

#include <innodb/tablespace/fil_system.h>

/** Rotate the tablespace keys by new master key.
@return true if the re-encrypt succeeds */
bool fil_encryption_rotate() {
    return (fil_system->encryption_rotate_all());
}
