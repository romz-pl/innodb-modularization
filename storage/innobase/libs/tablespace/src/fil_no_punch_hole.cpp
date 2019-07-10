#include <innodb/tablespace/fil_no_punch_hole.h>

#include <innodb/tablespace/fil_node_t.h>

/** Note that the file system where the file resides doesn't support PUNCH HOLE.
Called from AIO handlers when IO returns DB_IO_NO_PUNCH_HOLE
@param[in,out]	file		file to set */
void fil_no_punch_hole(fil_node_t *file) {
    file->punch_hole = false;
}
