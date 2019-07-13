#include <innodb/tablespace/fil_close_all_files.h>

#include <innodb/tablespace/fil_system.h>

/** Closes all open files. There must not be any pending i/o's or not flushed
modifications in the files. */
void fil_close_all_files() {
    fil_system->close_all_files();
}
