#pragma once

#include <innodb/univ/univ.h>

/** Free the log system data structures. Deallocate all the related memory. */
void log_sys_close();
