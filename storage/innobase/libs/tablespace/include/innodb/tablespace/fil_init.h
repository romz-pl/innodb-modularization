#pragma once

#include <innodb/univ/univ.h>

/** Initializes the tablespace memory cache.
@param[in]	max_n_open	Max number of open files. */
void fil_init(ulint max_n_open);

