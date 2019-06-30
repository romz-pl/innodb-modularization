#pragma once

#include <innodb/univ/univ.h>

/** Create the primary system wait array(s), they are protected by an OS mutex
 */
void sync_array_init(ulint n_threads); /*!< in: Number of slots to
                                      create in all arrays */
