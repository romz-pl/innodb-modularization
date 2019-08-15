#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Returns free space reserved for future updates of records. This is
 relevant only in the case of many consecutive inserts, as updates
 which make the records bigger might fragment the index.
 @return number of free bytes on page, reserved for updates */
UNIV_INLINE
ulint dict_index_get_space_reserve(void)
{
    return (UNIV_PAGE_SIZE / 16);
}
