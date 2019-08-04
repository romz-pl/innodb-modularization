#pragma once

#include <innodb/univ/univ.h>

struct hash_table_t;

/* Fix Bug #13859: symbol collision between imap/mysql */
#define hash_create hash0_create

/** Creates a hash table with >= n array cells. The actual number
 of cells is chosen to be a prime number slightly bigger than n.
 @return own: created table */
hash_table_t *hash_create(ulint n); /*!< in: number of array cells */
