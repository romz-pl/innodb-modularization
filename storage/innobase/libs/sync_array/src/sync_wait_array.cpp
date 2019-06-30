#include <innodb/sync_array/sync_wait_array.h>

/** The global array of wait cells for implementation of the database's own
mutexes and read-write locks */
sync_array_t **sync_wait_array;
