#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

#ifdef UNIV_HOTBACKUP

#define dict_lru_find_table(x) (true)

#else


#ifdef UNIV_DEBUG

/** Check if table is in the dictionary table LRU list.
 @return true if table found */
static ibool dict_lru_find_table(
    const dict_table_t *find_table); /*!< in: table to find */

#endif

#endif
