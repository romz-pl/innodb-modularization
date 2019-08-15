#pragma once

#include <innodb/univ/univ.h>

/** Releases the dictionary system mutex for MySQL. */
void dict_mutex_exit_for_mysql(void);
