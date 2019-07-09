#include <innodb/io/os_fsync_threshold.h>

/* Flush after each os_fsync_threshold bytes */
unsigned long long os_fsync_threshold = 0;
