#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <innodb/sync_array/sync_array_t.h>

/** Validates the integrity of the wait array. Checks
 that the number of reserved cells equals the count variable. */
void sync_array_validate(sync_array_t *arr); /*!< in: sync wait array */

#endif
