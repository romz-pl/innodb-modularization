#pragma once

#include <innodb/univ/univ.h>

/** Gets the next struct in a hash chain, NULL if none. */

#define HASH_GET_NEXT(NAME, DATA) ((DATA)->NAME)
