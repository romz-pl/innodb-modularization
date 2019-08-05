#pragma once

#include <innodb/univ/univ.h>

#include <innodb/hash/hash_get_nth_cell.h>
#include <innodb/hash/hash_cell_t.h>

/** Gets the first struct in a hash chain, NULL if none. */

#define HASH_GET_FIRST(TABLE, HASH_VAL) \
  (hash_get_nth_cell(TABLE, HASH_VAL)->node)
