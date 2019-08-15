#pragma once

#include <innodb/univ/univ.h>

/** Reserves the dictionary system mutex for MySQL. */
void dict_mutex_enter_for_mysql(void);
