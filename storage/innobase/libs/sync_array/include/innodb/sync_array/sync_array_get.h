#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_array_t.h>

/** Get an instance of the sync wait array. */
sync_array_t *sync_array_get();
