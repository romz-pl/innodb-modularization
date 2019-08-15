#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

#ifdef UNIV_HOTBACKUP

#define dict_non_lru_find_table(x) (true)

#else

#ifdef UNIV_DEBUG

/** Check if a table exists in the dict table non-LRU list.
 @return true if table found */
ibool dict_non_lru_find_table(
    const dict_table_t *find_table); /*!< in: table to find */

#endif

#endif
