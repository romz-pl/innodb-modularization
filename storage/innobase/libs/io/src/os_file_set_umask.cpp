#include <innodb/io/os_file_set_umask.h>

#include <innodb/io/os_innodb_umask.h>

/**
Set the file create umask
@param[in]	umask		The umask to use for file creation. */
void os_file_set_umask(ulint umask) {
    os_innodb_umask = umask;
}
