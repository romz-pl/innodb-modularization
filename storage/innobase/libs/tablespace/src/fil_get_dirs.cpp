#include <innodb/tablespace/fil_get_dirs.h>

#include <innodb/tablespace/fil_system.h>

/** Get the list of directories that datafiles can reside in.
@return the list of directories 'dir1;dir2;....;dirN' */
std::string fil_get_dirs() {
    return (fil_system->get_dirs());
}
