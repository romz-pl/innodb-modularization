#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_array_t.h>

/** Frees the resources in a wait array. */
void sync_array_free(sync_array_t *arr); /*!< in, own: sync wait array */
