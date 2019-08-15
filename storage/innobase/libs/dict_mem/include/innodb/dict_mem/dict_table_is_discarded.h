#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Check if the tablespace for the table has been discarded.
 @return true if the tablespace has been discarded. */
UNIV_INLINE
bool dict_table_is_discarded(
    const dict_table_t *table) /*!< in: table to check */
{
  return (DICT_TF2_FLAG_IS_SET(table, DICT_TF2_DISCARDED));
}
