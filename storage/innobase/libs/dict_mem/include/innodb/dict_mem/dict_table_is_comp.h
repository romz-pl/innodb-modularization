#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Check whether the table uses the compact page format.
 @return true if table uses the compact page format */
UNIV_INLINE
ibool dict_table_is_comp(const dict_table_t *table) /*!< in: table */
{
  ut_ad(table);

#if DICT_TF_COMPACT != 1
#error "DICT_TF_COMPACT must be 1"
#endif

  return (table->flags & DICT_TF_COMPACT);
}
